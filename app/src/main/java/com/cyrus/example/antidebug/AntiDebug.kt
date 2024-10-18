package com.cyrus.example.antidebug

import android.util.Log

object AntiDebug {

    init {
        // 加载 native 库
        System.loadLibrary("antidebug")
    }

    external fun detectDebugger(): Boolean

    fun isDebuggerDetected(): Boolean {
        val detected = detectDebugger()
        if (detected) {
            Log.i("AntiDebug", "Debugger detected!")
        } else {
            Log.i("AntiDebug", "No debugger detected.")
        }
        return detected
    }
}
