package com.mefazm.rdvr

import androidx.appcompat.app.AppCompatActivity

import android.os.Bundle

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
        sessionView?.post(Runnable() {
            if (null != hostname && null != username && null != password) {
                Log.i("hostname: \"$hostname\", username: \"$username\", password: \"$password\", home: \"${filesDir.path}\", w: \"${sessionView!!.width}\", h: \"${sessionView!!.height}\"")
                connect(hostname!!, username!!, password!!, filesDir!!.path, sessionView!!.width, sessionView!!.height)
            }
        })
    }

    private external fun connect(hostname: String, username: String, password: String, home: String, width: Int, height: Int)
    private external fun resize(width: Int, height: Int)

    companion object {
        init {
            System.loadLibrary("rdvr")
        }
    }

    private var hostname: String? = null
    private var username: String? = null
    private var password: String? = null
    private var sessionView: SessionView? = null
}
