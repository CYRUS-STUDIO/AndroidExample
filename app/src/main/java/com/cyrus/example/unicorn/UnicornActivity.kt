package com.cyrus.example.unicorn

import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R


class UnicornActivity : AppCompatActivity() {

    // 加载原生库
    companion object {
        init {
            System.loadLibrary("unicorn")
        }
    }

    // 声明 add 原生方法
    external fun add(a: Int, b: Int): Int

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_unicorn)

        // 加法运算
        val buttonAdd = findViewById<Button>(R.id.button_add)

        val a = 5
        val b = 3

        // 设置按钮文本
        buttonAdd.text = "$a + $b"

        buttonAdd.setOnClickListener {
            // 调用原生方法
            val result = add(a, b)
            // 显示 Toast
            Toast.makeText(this, "add() returned: $result", Toast.LENGTH_LONG).show()
        }
    }

}