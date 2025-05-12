package com.cyrus.example.hotfix

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
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

    fun getDexElementsFrom(classLoader: ClassLoader): Array<Any>? {
        return try {
            // 1. 拿到 pathList 字段
            val baseDexClassLoaderClass = Class.forName("dalvik.system.BaseDexClassLoader")
            val pathListField = baseDexClassLoaderClass.getDeclaredField("pathList")
            pathListField.isAccessible = true
            val pathList = pathListField.get(classLoader)

            // 2. 拿到 dexElements 字段
            val pathListClass = pathList.javaClass
            val dexElementsField = pathListClass.getDeclaredField("dexElements")
            dexElementsField.isAccessible = true
            @Suppress("UNCHECKED_CAST")
            dexElementsField.get(pathList) as? Array<Any>
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    fun mergeDexElements(first: Array<Any>, second: Array<Any>): Array<Any> {
        val elementClass = first.javaClass.componentType ?: throw IllegalArgumentException("first is not an array")

        val totalLength = first.size + second.size
        val result = java.lang.reflect.Array.newInstance(elementClass, totalLength) as Array<Any>

        // 拷贝数组
        System.arraycopy(first, 0, result, 0, first.size)
        System.arraycopy(second, 0, result, first.size, second.size)

        return result
    }

    @SuppressLint("DiscouragedPrivateApi")
    fun injectDexElementsToClassLoader(classLoader: ClassLoader, newDexElements: Array<Any>) {
        try {
            val pathListField = Class.forName("dalvik.system.BaseDexClassLoader")
                .getDeclaredField("pathList").apply { isAccessible = true }
            val pathList = pathListField.get(classLoader)

            val dexElementsField = pathList.javaClass
                .getDeclaredField("dexElements").apply { isAccessible = true }
            dexElementsField.set(pathList, newDexElements)

            Log.d(TAG, "✅ dexElements successfully replaced!")
        } catch (e: Exception) {
            e.printStackTrace()
            Log.d(TAG,"❌ Failed to inject dexElements")
        }
    }


    // 热修复
    fun hotFix() {
        val pathClassLoader = classLoader

        // 1. 创建自定义 ClassLoader 实例，加载 sdcard 上的 apk
        val classLoader = DexClassLoader(
            apkPath,
            null,
            this.packageResourcePath,
            pathClassLoader.parent
        )

        // 2. 通过反射拿到 ClassLoader 中的 dexElements 数组
        val baseDexElements = getDexElementsFrom(pathClassLoader)
        val pluginDexElements = getDexElementsFrom(classLoader)

        // 3. 合并两个 ClassLoader 中的 dexElements 数组
        val merged = mergeDexElements(pluginDexElements!!, baseDexElements!!)

        // 4. 替换掉 PathClassLoader 中的 dexElements
        injectDexElementsToClassLoader(pathClassLoader, merged)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            HotFixScreen(
                onHotFixClick = {
                    hotFix()
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
