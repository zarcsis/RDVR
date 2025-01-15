package com.mefazm.rdvr

import android.graphics.Bitmap
import androidx.appcompat.app.AppCompatActivity

import android.os.Bundle
import java.util.Timer
import java.util.TimerTask

class SessionActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("onCreate()")

        sessionView = SessionView(this)
        setContentView(sessionView!!)
        sessionView!!.setSessionSize { w: Int, h: Int ->
            resize(w, h)
        }
        if (null == hostname) hostname = intent.getStringExtra("hostname")!!
        if (null == username) username = intent.getStringExtra("username")!!
        if (null == password) password = intent.getStringExtra("password")!!
        sessionView!!.post(Runnable() {
            if (null != hostname && null != username && null != password) {
                Log.i("hostname: \"$hostname\", username: \"$username\", password: \"$password\", home: \"${filesDir.path}\", w: \"${sessionView!!.bmp!!.width}\", h: \"${sessionView!!.bmp!!.height}\"")
                connect(hostname!!, username!!, password!!, filesDir!!.path, sessionView!!.bmp!!)
                Timer().schedule(object : TimerTask() {
                    override fun run() {
                        sessionView!!.invalidate()
                    }
                }, 0, 1000 / 72)
            }
        })
    }

    override fun onStop() {
        super.onStop()
        Log.i("onStop()")
        disconnect()
    }

    private external fun connect(
        hostname: String,
        username: String,
        password: String,
        home: String,
        bmp: Bitmap
    )

    private external fun resize(width: Int, height: Int)
    private external fun disconnect()

    companion object {
        init {
            System.loadLibrary("rdvr")
            System.loadLibrary("freerdp-client3")
            System.loadLibrary("freerdp3")
            System.loadLibrary("winpr3")
        }
    }

    private var hostname: String? = null
    private var username: String? = null
    private var password: String? = null
    private var sessionView: SessionView? = null
}
