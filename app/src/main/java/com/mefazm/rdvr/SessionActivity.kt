package com.mefazm.rdvr

import androidx.appcompat.app.AppCompatActivity

import android.os.Bundle

class SessionActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("onCreate()")

        if (null == savedInstanceState) {
            Log.i("First start.")
        } else {
            Log.i("Restore.")
        }

        sessionView = SessionView(this)
        setContentView(sessionView!!)
        sessionView?.setSessionSize { w: Int, h: Int ->
            resize(w, h)
        }
        if (null == hostname) hostname = intent.getStringExtra("hostname")!!
        if (null == username) username = intent.getStringExtra("username")!!
        if (null == password) password = intent.getStringExtra("password")!!
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        Log.i("onSaveInstanceState()")
        outState.putBoolean("connected", connected)
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        super.onRestoreInstanceState(savedInstanceState)
        Log.i("onRestoreInstanceState()")
        connected = savedInstanceState.getBoolean("connected", false)
    }

    override fun onResume() {
        super.onResume()
        Log.i("onResume()")
        if (null != hostname && null != username && null != password && !connected) {
            Log.i("hostname: \"$hostname\", username: \"$username\", password: \"$password\", home: \"${filesDir.path}\"")
            connect(hostname!!, username!!, password!!, filesDir.path)
            connected = true
        }
    }

    private external fun connect(hostname: String, username: String, password: String, home: String)
    private external fun resize(width: Int, height: Int)

    companion object {
        init {
            System.loadLibrary("rdvr")
        }
    }

    private var hostname: String? = null
    private var username: String? = null
    private var password: String? = null
    private var connected = false
    private var sessionView: SessionView? = null
}
