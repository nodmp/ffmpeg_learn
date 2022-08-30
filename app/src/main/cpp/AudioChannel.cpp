//
// Created by Kyle on 2022/8/21.
//

#include "AudioChannel.h"

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
    LOGE("回调执行次数 + 1");

    auto *audioChannel = static_cast<AudioChannel *>(context);
    //获取pcm数据，多少字节
    int dataSize = audioChannel->getPcm();
    if (dataSize > 0) {
        //接受16位数据，所以获取的datasize是8位的大小，所以/2
        (*bq)->Enqueue(bq, audioChannel->buffer, dataSize / 2);
    }
}
void *audio_decode_task(void *arg){
    auto *audioChannel = static_cast<AudioChannel *>(arg);
    audioChannel->decode();
    return nullptr;
}

void *audio_play_task(void *arg){
    auto *audioChannel = static_cast<AudioChannel *>(arg);
    audioChannel->audio_play();
    return nullptr;
}
AudioChannel::AudioChannel(int index, AVCodecContext *pContext) : BaseChannel(index, pContext) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    buffer = static_cast<uint8_t *>(malloc(out_sample_rate * out_channels * out_samplesize));
    memset(buffer, 0, out_sample_rate * out_channels * out_samplesize);
}

AudioChannel::~AudioChannel() {
    if (buffer) {
        free(buffer);
    }
}

void AudioChannel::play() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    //0 + 输出声道 + 输出采样位 + 输出采样率
    swrContext = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                    avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate, 0, 0);
    swr_init(swrContext);
    pthread_create(&pid_audio_decode, 0, audio_decode_task, this);
    pthread_create(&pid_audio_play, 0, audio_play_task, this);
}

void AudioChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(&packet);
        if (ret != 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame); //从解码器中读取数据包
        if (ret == AVERROR(EAGAIN)) {
            continue; //需要更多的数据才能读出来这些数据
        } else if (ret != 0) {
            break;
        }
        LOGE("插入音频数据");
        frames.push(frame);
    }
    releaseAvPacket(&packet);

}

void AudioChannel::audio_play() {
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result){
        return;
    }
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result){
        return;
    }

    //设置混音器
    // create output mix, with environmental reverb specified as a non-required interface
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result){
        return;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result){
        return;
    }

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT |  SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                1, ids, req);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);



    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);


    //手动激活一下回调
    bqPlayerCallback(bqPlayerBufferQueue, this);

}

//返回pcm数据大小
int AudioChannel::getPcm() {
    int data_size = 0;
    AVFrame *frame = nullptr;
    int ret = frames.pop(frame);

    if (!isPlaying) {
        if (ret) {
            releaseAvFrame(&frame);
        }
        return data_size;
    }
    //将nbsameples个数据由sample_rate采样率转为44100后返回多少数据
    int64_t delays = swr_get_delay(swrContext, frame->sample_rate); //
    int64_t nb = av_rescale_rnd(delays + frame->nb_samples, out_sample_rate, frame->sample_rate, AV_ROUND_UP);
    //44100 * 2 (声道数) 获取数据
    int samples = swr_convert(swrContext, &buffer, nb,
                              const_cast<const uint8_t **>(frame->data),
                              frame->nb_samples);
    data_size =  samples * out_samplesize * out_channels ;
    return data_size;
    //重采样，
}

