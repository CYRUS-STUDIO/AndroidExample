package com.cyrus.example.classloader

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import dalvik.system.DexClassLoader
import java.io.File
import java.lang.ref.WeakReference

class ClassLoaderActivity : ComponentActivity() {

    private val TAG = "ClassLoaderActivity"

    @SuppressLint("SdCardPath")
    private val apkPath = "/sdcard/Android/data/com.cyrus.example/files/plugin-debug.apk"

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

        output = getActivityClassInfo(this@ClassLoaderActivity)

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
                // ClassLoader 链
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

                // 获取 ClasLoader 的类列表
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

                // 动态加载
                Button(onClick = {
                    val context = this@ClassLoaderActivity

                    // 方案 1：替换 ClassLoader 为 自定义ClassLoader
                    // val classLoader = replaceClassLoader(context)

                    // 方案 2：插入中间 ClassLoader / 打破双亲委派
                    val classLoader = injectClassLoader(context)

                    if (classLoader == null) {
                        Log.d(TAG, "❌ Failed to replace ClassLoader")
                        return@Button
                    }

                    // classLoader 加载 com.cyrus.example.plugin.PluginClass 类并通过反射调用 getString 方法
                    val pluginClass = classLoader.loadClass("com.cyrus.example.plugin.PluginClass")
                    val constructor = pluginClass.getDeclaredConstructor()
                    constructor.isAccessible = true
                    val instance = constructor.newInstance()
                    val method = pluginClass.getDeclaredMethod("getString")
                    method.isAccessible = true
                    val result = method.invoke(instance) as? String

                    // 通过 classLoader 加载 PluginActivity 类并启动
                    val pluginActivityClass = classLoader.loadClass("com.cyrus.example.plugin.PluginActivity")
                    val intent = Intent(this@ClassLoaderActivity, pluginActivityClass)
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    this@ClassLoaderActivity.startActivity(intent)

                    output = "动态加载：${apkPath}\n\ncall ${method}\n\nreuslt=${result}"
                }) {
                    Text("动态加载")
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


    private fun injectClassLoader(context: Context): DexClassLoader? {
        try {
            // 拿到当前 PathClassLoader
            val appClassLoader = context.classLoader
            val pathClassLoaderClass = ClassLoader::class.java

            // 反射访问 parent 字段
            val parentField = pathClassLoaderClass.getDeclaredField("parent")
            parentField.isAccessible = true

            val bootClassLoader = ClassLoader.getSystemClassLoader().parent

            // 自定义ClassLoader.parent = BootClassLoader
            val classLoader = DexClassLoader(
                apkPath,
                null,
                this@ClassLoaderActivity.packageResourcePath,
                bootClassLoader // 设置 parent 为 BootClassLoader
            )

            // PathClassLoader.parent = 自定义ClassLoader
            parentField.set(appClassLoader, classLoader)

            Log.d(TAG, "✅ 成功将 ${classLoader} 注入到 PathClassLoader.parent")

            return classLoader

        } catch (e: Exception) {
            e.printStackTrace()
            Log.d(TAG, "❌ 注入失败：${e.message}")
        }

        return null
    }

    private fun replaceClassLoader(context: Context): ClassLoader? {
        try {
            // 1. 创建自定义 ClassLoader 实例，加载 sdcard 上的 apk
            val classLoader = DexClassLoader(
                apkPath,
                null,
                this@ClassLoaderActivity.packageResourcePath,
                context.classLoader // 设置 parent 为 系统的 ClassLoader
            )

            // 2. 拿到 ActivityThread
            val activityThreadClass = Class.forName("android.app.ActivityThread")
            val currentActivityThread = activityThreadClass
                .getMethod("currentActivityThread")
                .invoke(null)

            // 3. 拿到 mPackages 字段: Map<String, WeakReference<LoadedApk>>
            val mPackagesField = activityThreadClass.getDeclaredField("mPackages")
            mPackagesField.isAccessible = true
            val mPackages = mPackagesField.get(currentActivityThread) as Map<*, *>

            // 4. 拿到当前包名对应的 LoadedApk 实例
            val loadedApkRef = mPackages[context.packageName] as? WeakReference<*>
                ?: throw IllegalStateException("LoadedApk not found for package: ${context.packageName}")
            val loadedApk = loadedApkRef.get()
                ?: throw IllegalStateException("LoadedApk is null")

            // 5. 替换 LoadedApk.mClassLoader
            val loadedApkClass = loadedApk.javaClass
            val mClassLoaderField = loadedApkClass.getDeclaredField("mClassLoader")
            mClassLoaderField.isAccessible = true
            mClassLoaderField.set(loadedApk, classLoader)

            // ✅ 替换成功
            Log.d(TAG, "✅ ClassLoader has been replaced successfully!")

            return classLoader

        } catch (e: Exception) {
            e.printStackTrace()
            Log.d(TAG, "❌ Failed to replace ClassLoader: ${e.message}")
        }

        return null
    }

    /**
     * 获取当前 Activity 的类信息和 ClassLoader 信息
     *
     * @param activity 当前 Activity 实例
     * @return 字符串描述类名和类加载器信息
     */
    fun getActivityClassInfo(activity: Activity): String {
        val clazz = activity::class.java
        val classLoader = clazz.classLoader
        return buildString {
            appendLine("Activity Class: ${clazz.name}")
            appendLine("ClassLoader: ${classLoader?.javaClass?.name}")
        }
    }

    /**
     * 使用指定的 ClassLoader 加载 Activity 并启动
     *
     * @param context       当前 Context
     * @param classLoader   要使用的 ClassLoader
     * @param className     要加载的 Activity 类名
     */
    fun startActivityFromClassLoader(
        context: Context,
        classLoader: ClassLoader,
        className: String
    ) {
        try {

        } catch (e: Exception) {
            e.printStackTrace()
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
