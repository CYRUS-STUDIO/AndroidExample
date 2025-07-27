package com.cyrus.example.sohooker


import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import marmojkfnf.mnhosc.cswoel.wvfxxn.Bhubscfh

class SoHookerActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val bhubscfh = Bhubscfh()

        setContent {

            var resultText by remember { mutableStateOf("点击按钮调用 native 方法") }

            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp)
            ) {

                Button(onClick = {
                    val result = bhubscfh.ca(this@SoHookerActivity)
                    resultText = "调用结果：$result"
                }) {
                    Text("调用 native 方法")
                }

                Spacer(modifier = Modifier.height(16.dp))

                Button(onClick = {
                    try {
                        System.loadLibrary("sohooker")
                        Toast.makeText(this@SoHookerActivity, "sohooker 加载成功", Toast.LENGTH_SHORT)
                            .show()
                    } catch (e: Throwable) {
                        e.printStackTrace()

                        Toast.makeText(
                            this@SoHookerActivity,
                            "sohooker 加载失败: ${e.message}",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                }) {
                    Text("加载 sohooker 篡改 so 函数返回值")
                }

                Spacer(modifier = Modifier.height(32.dp))

                Text(resultText, color = Color.White)
            }
        }
    }
}
