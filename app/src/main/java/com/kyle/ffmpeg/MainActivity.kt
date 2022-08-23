package com.kyle.ffmpeg

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.SurfaceView
import android.widget.TextView
import android.widget.Toast
import com.kyle.ffmpeg.databinding.ActivityMainBinding

private const val TAG = "MainActivity"
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private val surfaceView:SurfaceView by lazy {
        findViewById(R.id.surface)
    }
    private val player:Player by lazy {
        Player()
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.play.setOnClickListener{
            player.prepare()
        }
        player.surfaceView = surfaceView
//        player.dataSource =
//            "http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4"
        player.dataSource =
            "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8"
        player.setPrepareListener {
            runOnUiThread {
                Toast.makeText(this, "Ready!" + Thread.currentThread(), Toast.LENGTH_SHORT).show()
            }
            player.start()

        }
    }


}