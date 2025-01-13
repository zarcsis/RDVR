package com.mefazm.rdvr

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Bitmap.createBitmap
import android.graphics.Canvas
import android.view.View

class SessionView(context: Context) : View(context) {
    fun setSessionSize(f: (w: Int, h: Int) -> Unit) {
        setSessionSizeCall = f
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        Log.i("width: $width, height: $height")
        bmp = createBitmap(width, height, Bitmap.Config.ARGB_8888)
        canvas.drawBitmap(bmp!!, 0.0f, 0.0f, null)
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        Log.i("width: $w, height: $h")
        setSessionSizeCall?.invoke(w, h)
    }

    var bmp: Bitmap? = null

    private var setSessionSizeCall: ((Int, Int) -> Unit)? = null
}
