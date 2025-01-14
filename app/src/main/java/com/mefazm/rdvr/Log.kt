package com.mefazm.rdvr

class Log {
    companion object {
        fun i(msg: String) {
            val thread = Thread.currentThread()
            val stackElement = thread.stackTrace[3]
            val fullClassName = stackElement.className
            val className = fullClassName.substring(fullClassName.lastIndexOf(".") + 1)
            android.util.Log.i(
                className,
                "${stackElement.methodName}(tId: ${thread.id}, line: ${stackElement.lineNumber}): " + msg
            )
        }
    }
}
