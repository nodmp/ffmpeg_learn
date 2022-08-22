//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",__VA_ARGS__)

#define DELETE(obj) if(obj){ \
delete obj;                   \
obj = 0;                   \
}
#define THREAD_MAIN 1
#define THREAD_CHILD 2

//错误代码
//打不开视频
#define FFMPEG_CAN_NOT_OPEN_URL 1
//打不开流媒体
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
//打不开解码器
#define FFMPEG_FIND_DECODER_FAIL 3
//无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
//根据流信息，配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETRRS_FAIL 5
//打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL 6
//没有音视频
#define FFMPEG_NOMEDIA 7

#endif //FFMPEG_UTILS_H
