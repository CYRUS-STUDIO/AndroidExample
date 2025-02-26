package com.cyrus.example.unidbg

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
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
    }

    // 声明本地方法
    external fun add(a: Int, b: Int, c: Int, d: Int, e: Int, f: Int): Int
    external fun stringLength(str: String): Int


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_unidbg)

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

    }

}