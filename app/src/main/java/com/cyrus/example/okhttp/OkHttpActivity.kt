package com.cyrus.example.okhttp

import NetworkClient
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent

class OkHttpActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 启用日志拦截器
        NetworkClient.enableLogging()

        // 设置全局请求头（例如 Token）
        NetworkClient.setGlobalHeader("Authorization", "Bearer your_token")

        setContent {
            OkHttpDemoScreen()
        }
    }

}
