//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_BASECHANNEL_H
#define FFMPEG_BASECHANNEL_H

extern "C"{
#include <libavcodec/packet.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};
#include "safe_queue.h"

typedef void (*RenderFrame)(uint8_t *, int, int, int);
class BaseChannel {
public:
    BaseChannel(int index, AVCodecContext *avCodecContext) : index{index},
                                                             avCodecContext{avCodecContext} {
        frames.setReleaseCallback(BaseChannel::releaseAvFrame);
        packets.setReleaseCallback(BaseChannel::releaseAvPacket);
    }

    void setRenderFrameCallback(RenderFrame callback){
        this->callback = callback;
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();

    };

    static void releaseAvPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    static void releaseAvFrame(AVFrame **avframe) {
        if (avframe) {
            av_frame_free(avframe);
            *avframe = 0;
        }
    }

    //play -> 纯虚方法
    virtual void play() = 0;

    int isPlaying = 0;
    int index;

    AVCodecContext *avCodecContext;
    //解码数据队列，编码数据队列
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;

    RenderFrame callback;
};

#endif //FFMPEG_BASECHANNEL_H
