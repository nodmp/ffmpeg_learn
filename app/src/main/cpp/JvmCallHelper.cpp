//
// Created by Kyle on 2022/8/21.
//

#include "JvmCallHelper.h"
#include "utils.h"

//一旦涉及到jobject跨方法 跨线程，就需要创建全局引用
JvmCallHelper::JvmCallHelper(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm = vm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);
    jclass clazz = env->GetObjectClass(instance);
    error = env->GetMethodID(clazz, "onError", "(I)V");
    prepare = env->GetMethodID(clazz, "onPrepare", "()V");
}

JvmCallHelper::~JvmCallHelper() {
    env->DeleteGlobalRef(instance);
}

void JvmCallHelper::onError(int thread, int i) {
    //main thread
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(instance, error, i);
    } else {
        //子线程获取的情况
        JNIEnv *env;
        vm->AttachCurrentThread(&env, 0);
        env->CallVoidMethod(instance, error, i);
        vm->DetachCurrentThread();
    }
}

void JvmCallHelper::onPrepare(int thread) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(instance, prepare);
    } else {
        //子线程获取的情况
        JNIEnv *env;
        vm->AttachCurrentThread(&env, 0);
        env->CallVoidMethod(instance, prepare);
        vm->DetachCurrentThread();
    }
}
