package com.cyrus.example.shell

import android.content.Context
import com.cyrus.example.hotfix.Hotfix
import dalvik.system.InMemoryDexClassLoader
import java.nio.ByteBuffer

/**
 * 抽取壳
 */
object DexExtract {

    private var classLoader: InMemoryDexClassLoader? = null

    fun loadDex(context: Context): InMemoryDexClassLoader {

        if (classLoader == null) {

            // 1. 读取 assets 中的 dex 文件内容
            val dexBytes = context.assets.open("classes3_extracted.dex").use { inputStream ->
                inputStream.readBytes()
            }

            // 2. 包装成 ByteBuffer
            val dexBuffer = ByteBuffer.wrap(dexBytes)

            // 3. 创建 InMemoryDexClassLoader
            classLoader = InMemoryDexClassLoader(dexBuffer, context.classLoader)

            // 替换掉 PathClassLoader 中的 dexElements
            val pathClassLoader = context.classLoader
            Hotfix.hotfix(pathClassLoader, classLoader!!)
        }

        return classLoader!!
    }

    fun getString(): String? {
        // 通过反射加载目标类
        try {
            // classLoader 加载 com.cyrus.example.plugin.PluginClass 类并通过反射调用 getString 方法
            val pluginClass = classLoader!!.loadClass("com.cyrus.example.plugin.PluginClass")
            val constructor = pluginClass.getDeclaredConstructor()
            constructor.isAccessible = true
            val instance = constructor.newInstance()
            val method = pluginClass.getDeclaredMethod("getString")
            method.isAccessible = true
            return method.invoke(instance) as? String
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return null
    }

}

