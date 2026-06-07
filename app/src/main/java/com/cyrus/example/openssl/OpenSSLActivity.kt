package com.cyrus.example.openssl

import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R
import okhttp3.OkHttpClient
import okhttp3.Request
import kotlin.concurrent.thread

class OpenSSLActivity : AppCompatActivity() {

    private lateinit var editTextUrl: EditText
    private lateinit var buttonFetch: Button
    private lateinit var buttonFetchOkHttp: Button
    private lateinit var textViewResponse: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_openssl)

        System.loadLibrary("openssl-jni")

        editTextUrl = findViewById(R.id.edittext_url)
        buttonFetch = findViewById(R.id.button_fetch)
        buttonFetchOkHttp = findViewById(R.id.button_fetch_okhttp)
        textViewResponse = findViewById(R.id.textview_response)

        buttonFetch.setOnClickListener {
            val url = editTextUrl.text.toString()
            textViewResponse.text = "OpenSSL: 正在获取数据..."
            thread {
                val response = fetchHttpData(url)
                runOnUiThread {
                    textViewResponse.text = response
                }
            }
        }

        buttonFetchOkHttp.setOnClickListener {
            val url = editTextUrl.text.toString()
            textViewResponse.text = "OkHttp: 正在获取数据..."
            fetchDataWithOkHttp(url)
        }
    }

    private fun fetchDataWithOkHttp(url: String) {
        thread {
            val client = OkHttpClient()
            val request = Request.Builder()
                .url(url)
                .build()

            try {
                client.newCall(request).execute().use { response ->
                    val result = if (response.isSuccessful) {
                        response.body?.string() ?: "Empty body"
                    } else {
                        "Unexpected code $response"
                    }
                    runOnUiThread {
                        textViewResponse.text = result
                    }
                }
            } catch (e: Exception) {
                Log.e("OpenSSLActivity", "OkHttp error", e)
                runOnUiThread {
                    textViewResponse.text = "Error: ${e.message}"
                }
            }
        }
    }

    private external fun fetchHttpData(url: String): String
}
