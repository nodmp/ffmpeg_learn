//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_AUDIOCHANNEL_H
#define FFMPEG_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel: public BaseChannel{
public:
    AudioChannel(int index, AVCodecContext *pContext);

    ~AudioChannel();

    void play() override;
};


#endif //FFMPEG_AUDIOCHANNEL_H
