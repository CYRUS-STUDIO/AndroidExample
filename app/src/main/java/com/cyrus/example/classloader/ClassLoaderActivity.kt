package com.cyrus.example.classloader

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

class ClassLoaderActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            ClassLoaderScreen()
        }
    }

    @Composable
    fun ClassLoaderScreen() {

        val scrollState = rememberScrollState()

        var output by remember { mutableStateOf("") }

        Surface(
            modifier = Modifier.fillMaxSize(),
            color = MaterialTheme.colorScheme.background
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Button(onClick = {
                    // 打印并更新输出信息
                    val builder = StringBuilder()
                    var loader: ClassLoader? = this@ClassLoaderActivity::class.java.classLoader
                    var level = 0
                    while (loader != null) {
                        val line = "[$level] ${loader.javaClass.name}"
                        Log.d("ClassLoaderChain", line)
                        builder.appendLine(line)
                        loader = loader.parent
                        level++
                    }
                    output = builder.toString()
                }) {
                    Text("打印 ClassLoader 链")
                }

                Button(onClick = {
                    val classLoader = this@ClassLoaderActivity::class.java.classLoader
                    val classList = getAllClassesFromClassLoader(classLoader!!)
                    val builder = StringBuilder()
                    builder.appendLine("类加载器加载的类列表 (${classList.size} 个):")
                    classList.forEach {
                        Log.d("ClassList", it)
                        builder.appendLine(it)
                    }
                    output = builder.toString()
                }) {
                    Text("打印类列表")
                }

                // 类加载按钮
                Button(onClick = {
                    // 点击按钮时加载类并更新文本
                    val classLoader = this@ClassLoaderActivity.classLoader
                    val logMessages = StringBuilder()

                    // 第一次加载
                    logMessages.append(loadClassAndPrint(classLoader, "java.util.ArrayList"))

                    // 第二次加载
                    logMessages.append(loadClassAndPrint(classLoader, "java.util.ArrayList"))

                    // 自定义类加载
                    logMessages.append(loadClassAndPrint(classLoader, "com.cyrus.example.classloader.MyClass"))

                    // 再次加载自定义类
                    logMessages.append(loadClassAndPrint(classLoader, "com.cyrus.example.classloader.MyClass"))

                    // 更新输出文本
                    output = logMessages.toString()
                }) {
                    Text("类加载")
                }


                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f)
                        .verticalScroll(scrollState)
                        .padding(8.dp)
                ) {
                    Text(
                        text = output,
                        style = MaterialTheme.typography.bodyMedium
                    )
                }
            }
        }
    }

    /**
     * 类加载
     */
    private fun loadClassAndPrint(classLoader: ClassLoader, className: String): String {
        val logMessages = StringBuilder()
        try {
            // 加载类并获取它的类加载器
            val clazz = classLoader.loadClass(className)

            // 输出类加载器的信息
            logMessages.append("Class: $className\n")
            logMessages.append("Loaded by: ${clazz.classLoader}\n")
        } catch (e: ClassNotFoundException) {
            logMessages.append("Class not found: $className\n")
        }
        return logMessages.toString()
    }

    // 自定义内部类
    public class MyClass {
        // 空类
    }


    /**
     * 获取 ClassLoader 的类列表
     */
    @SuppressLint("DiscouragedPrivateApi")
    fun getAllClassesFromClassLoader(classLoader: ClassLoader): List<String> {
        val classNames = mutableListOf<String>()

        try {
            // 获取 BaseDexClassLoader 的 pathList 字段
            val pathListField = Class.forName("dalvik.system.BaseDexClassLoader")
                .getDeclaredField("pathList")
            pathListField.isAccessible = true
            val pathList = pathListField.get(classLoader)

            // 获取 pathList 中的 dexElements 字段
            val dexElementsField = pathList.javaClass.getDeclaredField("dexElements")
            dexElementsField.isAccessible = true
            val dexElements = dexElementsField.get(pathList) as Array<*>

            for (element in dexElements) {
                // 获取 dexElement 中的 dexFile 字段
                val dexFileField = element!!::class.java.getDeclaredField("dexFile")
                dexFileField.isAccessible = true
                val dexFile = dexFileField.get(element)

                val mCookieField = dexFile.javaClass.getDeclaredField("mCookie")
                mCookieField.isAccessible = true
                val mCookie = mCookieField.get(dexFile)

                // 调用 dexFile.getClassNameList()
                val getClassNameListMethod = dexFile.javaClass.getDeclaredMethod("getClassNameList", Any::class.java)
                getClassNameListMethod.isAccessible = true
                val result = getClassNameListMethod.invoke(dexFile, mCookie)

                // 将结果添加到列表中
                if (result is Array<*>) {
                    classNames.addAll(result.filterIsInstance<String>())
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }

        return classNames
    }


    /**
     * 打印 ClassLoader 链
     */
    private fun printClassLoaderChain() {
        var loader: ClassLoader? = this::class.java.classLoader
        var level = 0
        while (loader != null) {
            Log.d("ClassLoaderChain", "[$level] ${loader.javaClass.name}")
            loader = loader.parent
            level++
        }
        Log.d("ClassLoaderChain", "[$level] <bootstrap classloader> (null)")
    }

}
