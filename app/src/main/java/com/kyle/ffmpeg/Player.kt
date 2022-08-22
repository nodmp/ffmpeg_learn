package com.kyle.ffmpeg

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

private const val TAG = "Player"

class Player : SurfaceHolder.Callback {

    var listener: (() -> Unit)? = null;

    fun setPrepareListener(l: () -> Unit) {
        listener = l
    }
    var dataSource: String? = null
        set(value) {
            Log.e(TAG, "set dataSource ${value}")
            field = value
        }

    /**
     * start play
     */
    fun start() {
        native_start()
    }

    /**
     * stop play
     */
    fun stop() {

    }

    /**
     * release resource
     */
    fun release() {
        surfaceView?.holder?.removeCallback(this)
    }

    fun prepare() {
        dataSource?.let { native_prepare(it) }
    }

    var surfaceView: SurfaceView? = null
        set(value) {
            Log.e(TAG, "${surfaceView}")
            value?.holder?.addCallback(this)
            field = value
        }


    companion object {
        // Used to load the 'ffmpeg' library on application startup.
        init {
            System.loadLibrary("ffmpeg")
        }
    }

    //创建
    override fun surfaceCreated(holder: SurfaceHolder) {
        native_set_surface(holder.surface)
    }


    //配置发生改变
    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        native_set_surface(holder.surface)
    }

    //home
    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }

    //native
    external fun native_prepare(dataSource: String)

    external fun native_start()

    external fun native_set_surface(surface: Surface)
    //native callback
    fun onError(code: Int) {
        Log.e(TAG, "onError: ${code}")

    }


    fun onPrepare() {
        Log.e(TAG, "onPrepare: ")
        listener?.invoke()
    }
}