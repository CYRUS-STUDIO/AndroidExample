package com.cyrus.example.hotfix

import android.annotation.SuppressLint
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import dalvik.system.DexClassLoader
import java.lang.reflect.Method

class HotFixActivity : ComponentActivity() {

    private val TAG = "ClassLoaderActivity"

    @SuppressLint("SdCardPath")
    private val apkPath = "/sdcard/Android/data/com.cyrus.example/files/plugin-debug.apk"

    // 热修复
    fun hotfix() {
        val pathClassLoader = classLoader

        // 创建自定义 ClassLoader 实例，加载 sdcard 上的 apk
        val dexClassLoader = DexClassLoader(
            apkPath,
            null,
            this.packageResourcePath,
            pathClassLoader.parent
        )

        Hotfix.hotfix(pathClassLoader, dexClassLoader)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            HotFixScreen(
                onHotFixClick = {
                    hotfix()
                    "已加载热修复文件: $apkPath"
                },
                onPluginCall = {
                    try {
                        val clazz = Class.forName("com.cyrus.example.plugin.PluginClass")
                        val obj = clazz.getDeclaredConstructor().newInstance()
                        val method: Method = clazz.getDeclaredMethod("getString")
                        method.invoke(obj) as String
                    } catch (e: Exception) {
                        "调用失败: ${e.message}"
                    }
                }
            )
        }
    }
}

@Composable
fun HotFixScreen(
    onHotFixClick: () -> String,
    onPluginCall: () -> String
) {
    var output by remember { mutableStateOf("") }

    Surface(
        modifier = Modifier
            .fillMaxSize()
            .padding(10.dp),
        color = MaterialTheme.colorScheme.background
    ) {
        Column(
            modifier = Modifier.fillMaxSize(),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Button(onClick = { output = onHotFixClick() }) {
                Text("热修复")
            }
            Spacer(modifier = Modifier.height(16.dp))
            Button(onClick = { output = onPluginCall() }) {
                Text("PluginClass.getString")
            }
            Spacer(modifier = Modifier.height(24.dp))
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp)
            ) {
                Text(
                    text = output,
                    modifier = Modifier.align(Alignment.Center)
                )
            }
        }
    }
}
