//
// Created by Kyle on 2022/8/21.
//

#include "AudioChannel.h"

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    audioChannel->getPcm();
}
void *audio_decode_task(void *arg){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(arg);
    return nullptr;
}

void *audio_play_task(void *arg){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(arg);
    audioChannel->audio_play();
    return nullptr;
}
AudioChannel::AudioChannel(int index, AVCodecContext *pContext) : BaseChannel(index, pContext) {

}

AudioChannel::~AudioChannel() {

}

void AudioChannel::play() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_audio_decode, 0, audio_decode_task, this);
    pthread_create(&pid_audio_play, 0, audio_play_task, this);
}

void AudioChannel::decode() {
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

void AudioChannel::audio_play() {
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS == result){
        return;
    }
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result){
        return;
    }
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result){
        return;
    }

    //设置混音器
    // create output mix, with environmental reverb specified as a non-required interface
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS == result){
        return;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS == result){
        return;
    }

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT |  SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                1, ids, req);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);



    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);


    //手动激活一下回调
    bqPlayerCallback(bqPlayerBufferQueue, this);

}

//返回pcm数据大小
int AudioChannel::getPcm() {

}

