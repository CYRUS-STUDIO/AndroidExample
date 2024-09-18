package com.cyrus.example

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.antidebug.AntiDebugActivity

class MainActivity : AppCompatActivity() {

    // 声明native方法
    external fun getNativeString(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // 获取按钮并设置点击事件
        val debugButton: Button = findViewById(R.id.debugButton)
        debugButton.setOnClickListener {
            // 点击按钮后跳转到 AntiDebugActivity
            val intent = Intent(this, AntiDebugActivity::class.java)
            startActivity(intent)
        }

        // 加载native库
        System.loadLibrary("native-lib")

        val nativeButton = findViewById<Button>(R.id.native_button)
        nativeButton.setOnClickListener {
            // 调用native方法并显示Toast
            val message = getNativeString()
            Toast.makeText(this, message, Toast.LENGTH_LONG).show()
        }
    }
}
