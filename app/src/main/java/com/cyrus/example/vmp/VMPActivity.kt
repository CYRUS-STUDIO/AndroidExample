package com.cyrus.example.vmp

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R
import com.cyrus.vmp.SignUtil

class VMPActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_vmp)

        val input = "example"

        // Sign算法
        findViewById<Button>(R.id.button_sign).setOnClickListener {
            // 参数编码
            val signed = SignUtil.sign(input)
            // 显示 Toast
            Toast.makeText(this, signed, Toast.LENGTH_SHORT).show()
        }

        // Sign算法（VMP）
        findViewById<Button>(R.id.button_sign_vmp).setOnClickListener {
            // 模拟字节码输入
            val bytecode = byteArrayOf(0x1A, 0x00, 0x4E, 0x00) // const-string v0, "input"

            // 通过 VMP 解析器执行指令流
            val result = SimpleVMP.execute(bytecode, input)

            // 显示 Toast
            Toast.makeText(this, result, Toast.LENGTH_SHORT).show()
        }

        // const-string 指令
        findViewById<Button>(R.id.button_const_string).setOnClickListener {
            // 模拟字节码输入
            val bytecode = byteArrayOf(0x1A, 0x00, 0x4E, 0x00) // const-string v0, "input"

            // 通过 VMP 解析器执行指令流
            val result = SimpleVMP.execute(bytecode, input)

            // 显示 Toast
            Toast.makeText(this, result, Toast.LENGTH_SHORT).show()
        }

        // invoke-static 指令
        findViewById<Button>(R.id.button_invoke_static).setOnClickListener {

            // 模拟字节码输入
            val bytecode = byteArrayOf(
                0x1A, 0x00, 0x4E, 0x00,            // const-string v0, "input"
                0x71, 0x20, 0x20, 0x00, 0x05, 0x00 // invoke-static{v5, v0}, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object;Ljava/lang/String;)V
            )

            // 通过 VMP 解析器执行指令流
            val result = SimpleVMP.execute(bytecode, input)

            // 显示 Toast
            Toast.makeText(this, result, Toast.LENGTH_SHORT).show()
        }
    }

}