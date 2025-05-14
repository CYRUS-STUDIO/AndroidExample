package com.cyrus.example.hotfix

import android.annotation.SuppressLint
import android.util.Log
import dalvik.system.BaseDexClassLoader

object Hotfix {

    private val TAG = "Hotfix"

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
    fun hotfix(pathClassLoader: ClassLoader, dexClassLoader: BaseDexClassLoader) {
        // 1. 通过反射拿到 ClassLoader 中的 dexElements 数组
        val baseDexElements = getDexElementsFrom(pathClassLoader)
        val hotfixDexElements = getDexElementsFrom(dexClassLoader)

        // 2. 合并两个 ClassLoader 中的 dexElements 数组
        val merged = mergeDexElements(hotfixDexElements!!, baseDexElements!!)

        // 3. 替换掉 PathClassLoader 中的 dexElements
        injectDexElementsToClassLoader(pathClassLoader, merged)
    }

}