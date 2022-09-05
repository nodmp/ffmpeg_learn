package com.kyle.ffmpeg

import android.app.Application
import android.content.Context

class App : Application() {
    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        xcrash.XCrash.init(this)
    }

    override fun onCreate() {
        super.onCreate()

    }

}