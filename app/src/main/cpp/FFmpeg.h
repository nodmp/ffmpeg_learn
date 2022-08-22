//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_FFMPEG_H
#define FFMPEG_FFMPEG_H

extern  "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};
#include "utils.h"
#include "JvmCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"


class FFmpeg {
public:
    FFmpeg(JvmCallHelper *helper, const char *data_source);

    ~FFmpeg();

    void prepare();

    void start();

    void setRenderFrameCallback(RenderFrame callback);

private:
    char *data_source;
    AVFormatContext *formatContext;
    JvmCallHelper *helper;
    AudioChannel *audio_channel = nullptr;
    VideoChannel *video_channel = nullptr;
    int isPlaying;
    pthread_t pthread;
    pthread_t pid_play;
    RenderFrame callback = nullptr;
    friend void *play(void *args);
    friend void *task_prepare(void *args);

};


#endif //FFMPEG_FFMPEG_H
