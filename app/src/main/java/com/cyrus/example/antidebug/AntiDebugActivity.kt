package com.cyrus.example.antidebug

import android.os.Bundle
import android.os.Debug
import android.util.Log
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.File
import java.io.FileReader
import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket

class AntiDebugActivity : AppCompatActivity() {

    private val TAG = "AntiDebug"
    private lateinit var debugInfoTextView: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_anti_debug)

        // 绑定 TextView
        debugInfoTextView = findViewById(R.id.debugInfoTextView)

        // 使用协程来执行调试检测
        CoroutineScope(Dispatchers.Main).launch {
            val debugInfo = checkDebugInfo()

            // 将调试信息显示到 TextView
            debugInfoTextView.text = debugInfo

            // 打印调试信息到日志
            Log.d(TAG, debugInfo)
        }
    }

    // 检查所有的调试信息
    private suspend fun checkDebugInfo(): String {
        val debugInfoBuilder = StringBuilder()

        val debuggerConnected = isDebuggerConnected()
        val waitingForDebugger = isWaitingForDebugger()

        // 获取TrackerId（TracerPid）
        val tracerPid = hasTracerPid()
        // 从 /proc/self/stat 获取调试状态
        val debugStatus = getProcStatStatus()
        // 获取wchan trace标识
        val wchanStatus = getWchanStatus()

        // 检测 JDWP 端口时使用协程的 IO 线程
        val jdwpDetected = withContext(Dispatchers.IO) {
            detectJDWP()
        }

        debugInfoBuilder.append("Debugging Information:\n")
        debugInfoBuilder.append("Debugger Connected: ").append(debuggerConnected).append("\n")
        debugInfoBuilder.append("Waiting for Debugger: ").append(waitingForDebugger).append("\n")
        debugInfoBuilder.append("JDWP Port (Debugger Attached): ").append(jdwpDetected).append("\n")
        debugInfoBuilder.append("TracerPid: ").append(tracerPid).append("\n")
        debugInfoBuilder.append("状态: ").append(debugStatus).append("\n")
        debugInfoBuilder.append("Wchan 状态: ").append(wchanStatus).append("\n")

        if (debuggerConnected || waitingForDebugger || tracerPid != 0 || jdwpDetected) {
            debugInfoBuilder.append("\nApp is being debugged!\n")
        } else {
            debugInfoBuilder.append("\nApp is not being debugged.\n")
        }

        return debugInfoBuilder.toString()
    }

    // 方法 1: 使用 Debug.isDebuggerConnected()
    private fun isDebuggerConnected(): Boolean {
        return Debug.isDebuggerConnected()
    }

    // 方法 2: 检查 Debug.waitingForDebugger()
    private fun isWaitingForDebugger(): Boolean {
        return Debug.waitingForDebugger()
    }

    // 方法 3: 返回 TracerPid 的值
    private fun hasTracerPid(): Int {
        try {
            BufferedReader(FileReader("/proc/self/status")).use { reader ->
                var line: String?
                while (reader.readLine().also { line = it } != null) {
                    if (line!!.startsWith("TracerPid:")) {
                        return line!!.split(":")[1].trim().toInt()
                    }
                }
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return 0 // 如果没有找到 TracerPid，返回 0 表示没有被调试
    }

    // 方法 4: 检测调试端口（JDWP），在后台线程中运行
    private fun detectJDWP(): Boolean {
        return try {
            Socket().use { socket ->
                socket.connect(InetSocketAddress("127.0.0.1", 8700), 1000)
            }
            true
        } catch (e: IOException) {
            // 没有调试器连接
            false
        }
    }

    // 从 /proc/self/wchan 获取进程的等待状态
    private fun getWchanStatus(): String {
        try {
            // 读取 /proc/self/wchan 文件
            val wchanFile = File("/proc/self/wchan")
            if (wchanFile.exists()) {
                return wchanFile.readText().trim()
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return "无法获取 Wchan 状态"
    }


    // 解析 /proc/self/stat 获取进程状态
    private fun getProcStatStatus(): String {
        try {
            // 读取 /proc/self/stat 文件
            val statFile = File("/proc/self/stat")
            val statContent = statFile.readText()

            // /proc/self/stat 的内容格式是以空格分隔的字段
            // 第3个字段是进程状态
            val statFields = statContent.split(" ")
            if (statFields.size > 2) {
                val processState = statFields[2] // 进程状态字段
                return when (processState) {
                    "R" -> "运行中"
                    "S" -> "睡眠中"
                    "D" -> "不可中断睡眠中"
                    "T" -> "停止（可能是被调试状态）"
                    "Z" -> "僵尸进程"
                    else -> "未知状态: $processState"
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return "无法获取调试状态"
    }

}
