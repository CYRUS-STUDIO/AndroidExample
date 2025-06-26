package com.cyrus.example.okhttp

import NetworkClient
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch
import java.io.File

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

    @Composable
    fun OkHttpDemoScreen() {
        var responseText by remember { mutableStateOf("请求结果将显示在这里") }
        // Jetpack Compose 中用于 在 Composable 作用域内启动协程 的标准方式。
        val coroutineScope = rememberCoroutineScope()
        // 创建滚动状态
        val scrollState = rememberScrollState()

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp)
        ) {

            Button(onClick = {
                coroutineScope.launch {
                    try {
                        val result = NetworkClient.getSuspend("https://httpbin.org/get")
                        responseText = "GET 成功:\n$result"
                    } catch (e: Exception) {
                        responseText = "GET 失败: ${e.message}"
                    }
                }
            }) {
                Text("GET 请求")
            }

            Spacer(modifier = Modifier.height(8.dp))

            Button(onClick = {
                coroutineScope.launch {
                    try {
                        val json = """{"name": "Cyrus", "number": 16}"""
                        val result = NetworkClient.postJsonSuspend("https://httpbin.org/post", json)
                        responseText = "POST JSON 成功:\n$result"
                    } catch (e: Exception) {
                        responseText = "POST JSON 失败: ${e.message}"
                    }
                }
            }) {
                Text("POST JSON")
            }

            Spacer(modifier = Modifier.height(8.dp))

            Button(onClick = {
                coroutineScope.launch {
                    try {
                        val result = NetworkClient.postFormSuspend(
                            "https://httpbin.org/post",
                            formData = mapOf("username" to "admin", "password" to "123456")
                        )
                        responseText = "POST 表单 成功:\n$result"
                    } catch (e: Exception) {
                        responseText = "POST 表单 失败: ${e.message}"
                    }
                }
            }) {
                Text("POST 表单")
            }

            Spacer(modifier = Modifier.height(8.dp))

            Button(onClick = {
                coroutineScope.launch {
                    try {
                        val file = File(cacheDir, "example.txt").apply {
                            writeText("这是测试上传内容")
                        }

                        val result = NetworkClient.uploadFileSuspend(
                            url = "https://httpbin.org/post",
                            file = file,
                            fileField = "file"
                        )
                        responseText = "文件上传成功:\n$result"
                    } catch (e: Exception) {
                        responseText = "上传失败: ${e.message}"
                    }
                }
            }) {
                Text("上传文件")
            }

            Spacer(modifier = Modifier.height(16.dp))

            Text(
                text = responseText,
                color = Color.White,
                modifier = Modifier
                    .fillMaxWidth()
                    .weight(1f) // 让 Text 占据剩余空间（可滚动区域）
                    .verticalScroll(scrollState)
                    .padding(8.dp)
            )
        }
    }
}
