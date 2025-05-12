package com.cyrus.example

import android.app.Application
import android.content.Context
import com.cyrus.example.hook.CyrusStudioHook
import com.cyrus.example.root.RootManager

class CyrusStudioApplication : Application() {

    companion object {
        init {
            try {
                // Dex2C
                System.loadLibrary("nc")
            } catch (e: Throwable) {
                // 加载失败，不处理
            }
        }
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        CyrusStudioHook.init()
        CyrusStudioHook.hookExecve()
    }

    override fun onCreate() {
        super.onCreate()
        RootManager.init()
    }

}
