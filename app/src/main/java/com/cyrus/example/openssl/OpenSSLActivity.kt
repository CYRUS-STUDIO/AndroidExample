package com.cyrus.example.openssl

import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R
import kotlin.concurrent.thread

class OpenSSLActivity : AppCompatActivity() {

    private lateinit var editTextUrl: EditText
    private lateinit var buttonFetch: Button
    private lateinit var textViewResponse: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_openssl)

        System.loadLibrary("openssl-jni")

        editTextUrl = findViewById(R.id.edittext_url)
        buttonFetch = findViewById(R.id.button_fetch)
        textViewResponse = findViewById(R.id.textview_response)

        buttonFetch.setOnClickListener {
            val url = editTextUrl.text.toString()
            textViewResponse.text = "正在获取数据..."
            thread {
                val response = fetchHttpData(url)
                runOnUiThread {
                    textViewResponse.text = response
                }
            }
        }
    }

    private external fun fetchHttpData(url: String): String
}
