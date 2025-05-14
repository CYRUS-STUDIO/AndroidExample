package com.cyrus.example.shell

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.cyrus.example.hook.CyrusStudioHook

class DexExtractActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            DexExtractScreen()
        }
    }

    @Composable
    fun DexExtractScreen() {
        var output by remember { mutableStateOf("") }

        // 布局：垂直居中对齐，按钮间距和外边距为 12dp
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(12.dp),
            verticalArrangement = Arrangement.Top,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {

            Button(
                onClick = {
                    output += "Clicked: hook LoadMethod，自动回填 CodeItem\n"
                    CyrusStudioHook.hookLoadMethod()
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(bottom = 12.dp)
            ) {
                Text("hook LoadMethod，自动回填 CodeItem")
            }

            Button(
                onClick = {
                    output += "Clicked: 加载 Dex\n"
                    DexExtract.loadDex(this@DexExtractActivity)
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(bottom = 12.dp)
            ) {
                Text("加载 Dex")
            }

            Button(
                onClick = {
                    output += "Clicked: 调用 getString\n"
                    output += "返回结果：${DexExtract.getString()}\n"
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(bottom = 12.dp)
            ) {
                Text("加载 PluginClass 并调用 getString")
            }

            Spacer(modifier = Modifier.height(12.dp))

            Text(
                text = output,
                color = Color.White,
                modifier = Modifier
                    .fillMaxSize()
                    .padding(12.dp)
                    .verticalScroll(rememberScrollState())
            )
        }
    }
}
