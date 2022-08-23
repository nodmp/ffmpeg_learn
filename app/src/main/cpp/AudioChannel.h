//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_AUDIOCHANNEL_H
#define FFMPEG_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class AudioChannel: public BaseChannel{
public:
    AudioChannel(int index, AVCodecContext *pContext);

    ~AudioChannel();

    void play() override;

    void decode();


    void audio_play();

    int getPcm();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;
    SLObjectItf outputMixObject = nullptr;
    SLObjectItf bqPlayerObject = nullptr;
    SLPlayItf bqPlayerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = nullptr;

};


#endif //FFMPEG_AUDIOCHANNEL_H
