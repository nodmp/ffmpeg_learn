//
// Created by Kyle on 2022/8/21.
//

#include "FFmpeg.h"
#include <cstring>
#include <pthread.h>



FFmpeg::~FFmpeg() {
    DELETE(data_source)
    DELETE(helper)
}

void *task_prepare(void *args){
    FFmpeg *fFmpeg = static_cast<FFmpeg *>(args);
    //0.初始化网络，让ffmpeg能够使用网络
    avformat_network_init();
    fFmpeg->formatContext = avformat_alloc_context(); //? nullptr
    int ret = avformat_open_input(&fFmpeg->formatContext, fFmpeg->data_source, 0, 0);
    if (ret) {
        LOGE("打开url失败:%s", av_err2str(ret));
        fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return nullptr;
    }
    //2.查找媒体中的音视频流
    ret = avformat_find_stream_info(fFmpeg->formatContext, 0);
    if (ret < 0) {
        LOGE("查找视频流失败:%s", av_err2str(ret));
        fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return nullptr;
    }
    for (int i = 0; i < fFmpeg->formatContext->nb_streams; ++i) {
        //可能代表是一个视频，也可能代表一个音频
        AVStream *stream = fFmpeg->formatContext->streams[i];
        AVCodecParameters *codecParameters = stream->codecpar;
        //通用的处理，直接获取解码器
        //1.通过当前流使用的编码方式，查找解码器
        auto *dec = const_cast<AVCodec *>(avcodec_find_decoder(codecParameters->codec_id));
        if (dec == nullptr) {
            LOGE("获取解码器失败:%s", av_err2str(ret));
            fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return nullptr;
        }
        //2.获取解码器上下文
        auto *context = avcodec_alloc_context3(dec);
        if (nullptr == context) {
            LOGE("获取解码器上下文失败:%s", av_err2str(ret));
            fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return nullptr;

        }
        //3.设置上下文类型参数
        ret = avcodec_parameters_to_context(context, codecParameters);
        if (ret < 0) {
            LOGE("设置上下文参数失败:%s", av_err2str(ret));
            fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETRRS_FAIL);
            return nullptr;
        }
        //4.打开编码器
        ret = avcodec_open2(context, dec, 0);
        if (ret != 0) {
            fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return nullptr;
        }
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGE("获取了一段音频");
            fFmpeg->audio_channel = new AudioChannel(i, context);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGE("获取了一段视频");
            fFmpeg->video_channel = new VideoChannel(i, context);
            //todo 这里需要在prepare之前设置callback
            if (fFmpeg->callback) {
                fFmpeg->video_channel->setRenderFrameCallback(fFmpeg->callback);
            }
        }

    }

    if (!fFmpeg->video_channel && !fFmpeg->audio_channel) {
        fFmpeg->helper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return nullptr;
    }
    fFmpeg->helper->onPrepare(THREAD_CHILD);
    return nullptr;
}

void FFmpeg::prepare() {
    pthread_create(&pthread, 0, task_prepare, this);
}

FFmpeg::FFmpeg(JvmCallHelper *helper, const char *data_source) {
    this->helper = helper;
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);
}
//in thread create
void *play(void *args){
    auto *ffmpeg = static_cast<FFmpeg *>(args);
    int ret;
    while (ffmpeg->isPlaying) {
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(ffmpeg->formatContext, packet);
        if (ret == 0) {
            if (ffmpeg->video_channel && packet->stream_index == ffmpeg->video_channel->index) {
//                LOGE("创建了一个队列并插入");
                ffmpeg->video_channel->packets.push(packet);
            } else if (ffmpeg->audio_channel &&
                       packet->stream_index == ffmpeg->audio_channel->index) {
                ffmpeg->audio_channel->packets.push(packet);
            } else {
                LOGE("未知包，未处理");
            }
        } else if (ret == AVERROR_EOF) { //读取完成，但是还没完成播放
            //

        } else {

        }
    }
    return nullptr;
}
void FFmpeg::start() {
    isPlaying = 1;
    if (video_channel) {
        video_channel->play();
    }
    //声音解码与播放
    if (audio_channel) {
        audio_channel->play();
    }
    pthread_create(&pid_play, 0, play, this);
}

void FFmpeg::setRenderFrameCallback(RenderFrame callback) {
    this->callback = callback;
}
