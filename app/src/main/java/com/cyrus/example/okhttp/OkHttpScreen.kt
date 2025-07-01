package com.cyrus.example.okhttp

import NetworkClient
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch
import java.io.File

@Composable
fun OkHttpDemoScreen() {
    val context = LocalContext.current
    val cacheDir = context.cacheDir

    var responseText by remember { mutableStateOf("请求结果将显示在这里") }
    val coroutineScope = rememberCoroutineScope()
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
                .weight(1f)
                .verticalScroll(scrollState)
                .padding(8.dp)
        )
    }
}
