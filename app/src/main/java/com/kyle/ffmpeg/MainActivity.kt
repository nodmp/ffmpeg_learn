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
//            "https://ws.flv.huya.com/src/1075804701-1075804701-4620546007678058496-2151732858-10057-A-0-1.flv?wsSecret=aa1573035d3607dc584c95b19627daee&wsTime=63064ec3&fm=RFdxOEJjSjNoNkRKdDZUWV8kMF8kMV8kMl8kMw%3D%3D&ctype=tars_mobile&fs=bgct&sphdcdn=al_7-tx_3-js_3-ws_7-bd_2-hw_2&sphdDC=huya&sphd=264_*-265_*&exsphd=264_500,264_2000,264_4000,&t=103"
//        player.dataSource =
//            "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8"
        player.dataSource = "http://vfx.mtime.cn/Video/2019/03/12/mp4/190312143927981075.mp4"
        player.setPrepareListener {
            runOnUiThread {
                Toast.makeText(this, "Ready!" + Thread.currentThread(), Toast.LENGTH_SHORT).show()
            }
            player.start()

        }
    }


}