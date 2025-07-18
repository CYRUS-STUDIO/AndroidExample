package com.cyrus.example.retrofit

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.Button
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.cyrus.example.retrofit.network.RetrofitClient
import com.cyrus.example.retrofit.network.request
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class RetrofitActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            RetrofitDemoScreen()
        }
    }
}

@Composable
fun RetrofitDemoScreen() {
    val coroutineScope = rememberCoroutineScope()
    var resultText by remember { mutableStateOf("🔽 请求结果显示区域") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {

        Text("Retrofit 示例", style = MaterialTheme.typography.h6, color = Color.White)

        Spacer(modifier = Modifier.height(16.dp))

        Button(onClick = {
            CoroutineScope(Dispatchers.IO).launch {
                try {
                    val user = RetrofitClient.api.getUser(1)
                    resultText = "用户1: ${user.name} (${user.email})"
                } catch (e: Exception) {
                    resultText = "请求失败: ${e.message}"
                }
            }
        }, modifier = Modifier.fillMaxWidth()) {
            Text("获取用户 1")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {

            request {
                RetrofitClient.api.getUser(2)
            }.onSuccess {
                resultText = "用户2: ${it.name} (${it.email})"
            }.onFailure {
                resultText = "❌ 请求失败：${it.message}"
            }.launch(coroutineScope)

        }, modifier = Modifier.fillMaxWidth()) {
            Text("获取用户 2")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {

            request {
                RetrofitClient.api.getPlainText()
            }.onSuccess {
                resultText = it
            }.onFailure {
                resultText = "❌ 请求失败：${it.message}"
            }.launch(coroutineScope)

        }, modifier = Modifier.fillMaxWidth()) {
            Text("getPlainText")
        }

        Spacer(modifier = Modifier.height(32.dp))

        Text(text = resultText, color = Color.White)
    }
}
