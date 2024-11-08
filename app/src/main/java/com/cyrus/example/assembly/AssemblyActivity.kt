package com.cyrus.example.assembly

import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R


/**
 * 内联汇编
 */
class AssemblyActivity : AppCompatActivity() {

    // 加载 native 库
    init {
        System.loadLibrary("assembly-lib");
    }

    // 通过内联汇编实现的加密函数
    external fun encryptString(input: String?): String

    // 通过内联汇编实现的解密函数
    external fun decryptString(input: String?): String

    // 声明 native 方法
    external fun addNumbers(a: Int, b: Int): Int

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_assembly) // 更新布局文件名

        // 原始字符串
        val input = "Hello, 内联汇编!"

        // 加密按钮
        val encryptButton = findViewById<Button>(R.id.button_encrypt)
        encryptButton.setOnClickListener { view: View? ->
            // 调用 C++ 方法获取加密后的字符串
            val encrypted = encryptString(input)

            // 打印原字符串和加密后的字符串
            val message = "Original: $input\nEncrypted: $encrypted"
            Toast.makeText(this@AssemblyActivity, message, Toast.LENGTH_LONG).show()
        }

        // 解密按钮
        val decryptButton = findViewById<Button>(R.id.button_decrypt)
        decryptButton.setOnClickListener { view: View? ->

            // 调用 C++ 方法获取加密后的字符串
            val encrypted = encryptString(input)

            val decrypted = decryptString(encrypted)

            // 打印加密字符串和解密后的字符串
            val message = "Encrypted: $encrypted\nDecrypted: $decrypted"
            Toast.makeText(this@AssemblyActivity, message, Toast.LENGTH_LONG).show()
        }

        // 兼容不同的 CPU 架构
        findViewById<Button>(R.id.button_compatible).setOnClickListener {
            // 传入两个 double 类型的值
            val a = 1
            val b = 2
            val result = addNumbers(a, b)  // 调用 JNI 方法进行浮点加法运算

            // 显示加法运算的结果
            Toast.makeText(this@AssemblyActivity, "Result of $a + $b = $result", Toast.LENGTH_LONG).show()
        }

    }
}