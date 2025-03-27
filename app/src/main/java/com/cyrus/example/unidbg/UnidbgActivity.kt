package com.cyrus.example.unidbg

import android.os.Bundle
import android.util.Base64
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R

class UnidbgActivity : AppCompatActivity() {

    companion object {
        // 加载原生库
        init {
            System.loadLibrary("unidbg")
        }

        // 声明静态 native 方法
        @JvmStatic
        external fun staticAdd(a: Int, b: Int, c: Int, d: Int, e: Int, f: Int): Int

        // 静态变量 a
        var a: String? = null
    }

    // 非静态变量 b
    var b: String? = null

    // 声明本地方法
    external fun add(a: Int, b: Int, c: Int, d: Int, e: Int, f: Int): Int
    external fun stringLength(str: String): Int

    // Base64 方法
    fun base64(content: String): String {
        return Base64.encodeToString(content.toByteArray(), Base64.NO_WRAP)
    }

    // Native 方法 sign
    external fun sign(content: String): String


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_unidbg)

        // 初始化静态变量 a 和 非静态变量 b
        a = "StaticA"
        b = "NonStaticB"

        // add
        findViewById<Button>(R.id.button_add).setOnClickListener {
            val result = add(1, 2, 3, 4, 5, 6)
            // 显示 Toast
            Toast.makeText(this, "add() returned: $result", Toast.LENGTH_SHORT).show()
        }

        // stringLength
        findViewById<Button>(R.id.button_string_length).setOnClickListener {
            val str = "hello"
            val result = stringLength(str)
            // 显示 Toast
            Toast.makeText(this, "stringLength($str) returned: $result", Toast.LENGTH_SHORT).show()
        }

        // base64
        findViewById<Button>(R.id.button_sign).setOnClickListener {
            val str = "hello"
            val result = sign(str)
            // 显示 Toast
            Toast.makeText(this, "sign($str) returned: $result", Toast.LENGTH_SHORT).show()
        }

    }

}