package com.cyrus.example.frida.disassemble

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R

/**
 * Frida 反汇编
 */
class FridaDisassembleActivity : AppCompatActivity() {

    companion object {
        init {
            System.loadLibrary("frida_disassemble") // 加载 native 库
        }
    }

    external fun add(): Int

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_frida_disassemble)

        // 加法运算
        findViewById<Button>(R.id.button_add).setOnClickListener {
            // 调用 native 方法
            val result = add()
            // 显示 Toast
            Toast.makeText(this, "add() returned: $result", Toast.LENGTH_LONG).show()
        }

    }

}