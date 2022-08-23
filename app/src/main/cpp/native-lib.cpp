#include <jni.h>
#include <string>
#include <android/log.h>
#include "FFmpeg.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C"{
#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libavfilter/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>
#include <libavformat/avformat.h>
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define TAG "native_ffmpeg"
ANativeWindow *window;
FFmpeg *ffmpeg = nullptr;
JavaVM *g_vm = nullptr;

void RenderMethodCallback(uint8_t *data, int line, int w, int h) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&mutex);
        return;
    }
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4; //
    // one by one
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, data + i * line, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}
int JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_vm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kyle_ffmpeg_Player_native_1prepare(JNIEnv *env, jobject thiz, jstring data_source) {
    const char *url = env->GetStringUTFChars(data_source, 0);
    auto *helper = new JvmCallHelper(g_vm, env, thiz);
    ffmpeg = new FFmpeg(helper, url); //Init url
    ffmpeg->setRenderFrameCallback(RenderMethodCallback);
    ffmpeg->prepare();
    env->ReleaseStringChars(data_source, reinterpret_cast<const jchar *>(url));
}
extern "C"
JNIEXPORT void JNICALL
Java_com_kyle_ffmpeg_Player_native_1start(JNIEnv *env, jobject thiz) {
    ffmpeg->start();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_kyle_ffmpeg_Player_native_1set_1surface(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

