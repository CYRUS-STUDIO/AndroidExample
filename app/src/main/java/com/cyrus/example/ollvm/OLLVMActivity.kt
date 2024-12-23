package com.cyrus.example.ollvm

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R

/**
 * 移植 OLLVM 到 Android NDK
 */
class OLLVMActivity : AppCompatActivity() {

    // 声明 native 方法
    external fun sub(a: Int, b: Int): Int
    external fun bcf(input: String?): String?
    external fun fla(x: Int, y: Int): String?

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_ollvmactivity)
        // 加载本地库
        System.loadLibrary("ollvm-lib");


        // 调用 native 方法并显示结果
        val textView = findViewById<TextView>(R.id.textView)

        val subResult = sub(10, 5)
        val bcfResult = bcf("Hello OLLVM!")
        val flaResult = fla(3, 2)

        val resultText = """
            sub(10, 5) = $subResult
            bcf("Hello OLLVM!") = $bcfResult
            fla(x, y) = $flaResult
            """.trimIndent()

        textView.text = resultText
    }

}