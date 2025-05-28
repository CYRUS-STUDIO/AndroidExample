package com.cyrus.example.fart

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
import androidx.compose.ui.unit.sp
import dalvik.system.DexClassLoader

/**
 * https://github.com/CYRUS-STUDIO/FART
 */
class FartActivity : ComponentActivity() {

    var mClassLoader: DexClassLoader? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            FartScreen()
        }
    }

    @Composable
    fun FartScreen() {
        var outputText by remember { mutableStateOf("") }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Button(
                onClick = { onDynamicDexLoadClicked { outputText += it + "\n" } },
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 6.dp)
            ) {
                Text(text = "动态加载 dex", fontSize = 16.sp)
            }

            Button(
                onClick = { onLocalClassLoaderClicked { outputText += it + "\n" } },
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 6.dp)
            ) {
                Text(text = "局部变量的 ClassLoader", fontSize = 16.sp)
            }

            Spacer(modifier = Modifier.height(16.dp))

            Text(
                text = outputText,
                modifier = Modifier
                    .fillMaxWidth()
                    .weight(1f) // 占满剩余空间
                    .verticalScroll(rememberScrollState())
                    .padding(8.dp),
                fontSize = 14.sp,
                color = Color.White
            )
        }
    }


    /**
     * 动态加载 dex
     */
    fun onDynamicDexLoadClicked(log: (String) -> Unit) {
        val apkPath = "/sdcard/Android/data/com.cyrus.example/files/plugin-debug.apk"

        // 创建 DexClassLoader 加载 sdcard 上的 apk
        val classLoader = DexClassLoader(
            apkPath,
            null,
            this@FartActivity.packageResourcePath,
            classLoader // parent 设为当前 context 的类加载器
        )

        // classLoader 加载 com.cyrus.example.plugin.FartTest 类并通过反射调用 test 方法
        val pluginClass = classLoader.loadClass("com.cyrus.example.plugin.FartTest")
        val constructor = pluginClass.getDeclaredConstructor()
        constructor.isAccessible = true
        val instance = constructor.newInstance()
        val method = pluginClass.getDeclaredMethod("test")
        method.isAccessible = true
        val result = method.invoke(instance) as? String

        log("动态加载：${apkPath}\n\ncall ${method}\n\nreuslt=${result}\n\n")

        mClassLoader = classLoader
    }


    /**
     * 局部变量的 ClassLoader
     */
    fun onLocalClassLoaderClicked(log: (String) -> Unit) {

        val apkPath = "/sdcard/Android/data/com.cyrus.example/files/plugin-debug.apk"

        // 创建 DexClassLoader 加载 sdcard 上的 apk
        val classLoader = DexClassLoader(
            apkPath,
            null,
            this@FartActivity.packageResourcePath,
            classLoader // parent 设为当前 context 的类加载器
        )

        // classLoader 加载 com.cyrus.example.plugin.FartTest 类并通过反射调用 test 方法
        val pluginClass = classLoader.loadClass("com.cyrus.example.plugin.FartTest")
        val constructor = pluginClass.getDeclaredConstructor()
        constructor.isAccessible = true
        val instance = constructor.newInstance()
        val method = pluginClass.getDeclaredMethod("test")
        method.isAccessible = true
        val result = method.invoke(instance) as? String

        log("局部变量的 ClassLoader 动态加载：${apkPath}\n\ncall ${method}\n\nreuslt=${result}\n\n")
    }

}
