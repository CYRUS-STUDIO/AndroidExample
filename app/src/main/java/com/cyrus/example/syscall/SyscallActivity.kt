package com.cyrus.example.syscall

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R

class SyscallActivity : AppCompatActivity() {

    // 加载 native 库
    init {
        System.loadLibrary("syscall-lib")
    }

    external fun readFileWithSyscall(path: String): String

    external fun readFileWithAssemblySyscall(path: String): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_syscall)

        // 指定文件路径
        val filePath = "/data/local/tmp/test.txt"

        findViewById<Button>(R.id.button_syscall).setOnClickListener {

            // 调用 native 方法读取文件内容
            val fileContent = readFileWithSyscall(filePath)

            // 显示 Toast
            Toast.makeText(this, fileContent, Toast.LENGTH_SHORT).show()
        }

        findViewById<Button>(R.id.button_assembly_syscall).setOnClickListener {

            // 调用 native 方法读取文件内容
            val fileContent = readFileWithAssemblySyscall(filePath)

            // 显示 Toast
            Toast.makeText(this, fileContent, Toast.LENGTH_SHORT).show()
        }
    }

}