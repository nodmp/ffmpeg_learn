//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_VIDEOCHANNEL_H
#define FFMPEG_VIDEOCHANNEL_H


#include "BaseChannel.h"
/**
 * 1.解码
 * 2.播放
 */

class VideoChannel: public BaseChannel{
public:
    VideoChannel(int index, AVCodecContext *pContext);

    ~VideoChannel();

    void play() override;

    void decode();

    void render();

private:
    pthread_t pid_decode;
    pthread_t pid_render;

    SwsContext *swsContext = nullptr;
};


#endif //FFMPEG_VIDEOCHANNEL_H
