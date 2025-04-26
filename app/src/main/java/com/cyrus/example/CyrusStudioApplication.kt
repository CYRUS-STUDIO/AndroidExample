package com.cyrus.example

import android.app.Application

class CyrusStudioApplication : Application() {

    companion object {
        init {
            try {
                System.loadLibrary("nc")
            } catch (e: Throwable) {
                // 加载失败，不处理
            }
        }
    }

}
