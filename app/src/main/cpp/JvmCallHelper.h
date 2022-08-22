//
// Created by Kyle on 2022/8/21.
//

#ifndef FFMPEG_JVMCALLHELPER_H
#define FFMPEG_JVMCALLHELPER_H

#include <jni.h>

class JvmCallHelper {
public:
    JvmCallHelper(JavaVM *vm, JNIEnv *env, jobject instance);
    ~JvmCallHelper();

    void onError(int thread, int i);

    void onPrepare(int thread);

private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
//method id
    jmethodID error;
    jmethodID prepare;
};


#endif //FFMPEG_JVMCALLHELPER_H
