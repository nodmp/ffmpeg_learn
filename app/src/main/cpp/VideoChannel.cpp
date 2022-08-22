//
// Created by Kyle on 2022/8/21.
//

#include "VideoChannel.h"

void *decode_task(void *args) {
    auto *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    return nullptr;
}

void *render_task(void *args) {
    auto *channel = static_cast<VideoChannel *>(args);
    channel->render();
    return nullptr;
}

VideoChannel::VideoChannel(int index, AVCodecContext *pContext) : BaseChannel(index,pContext) {

}

VideoChannel::~VideoChannel() {

}

void VideoChannel::play() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_decode, 0, decode_task, this);
    pthread_create(&pid_render, 0, render_task, this);
}

void VideoChannel::decode() {
    //调用decode方法，然后从队列里面取数据并解码
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
        //重新开一个线程，来播放数据
        frames.push(frame);
    }
    releaseAvPacket(&packet);
}

void VideoChannel::render() {
    swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                avCodecContext->pix_fmt, avCodecContext->width,
                                avCodecContext->height, AV_PIX_FMT_RGBA,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);
    AVFrame *frame = nullptr;
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, avCodecContext->height, dst_data,
                  dst_linesize);

        callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        releaseAvFrame(&frame);
    }
    av_freep(&dst_data[0]);

    releaseAvFrame(&frame);

}
