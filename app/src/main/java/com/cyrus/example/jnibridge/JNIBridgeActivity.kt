package com.cyrus.example.jnibridge

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R

class JNIBridgeActivity : AppCompatActivity() {

    interface ICallback {
        fun onCall(msg: String): String
    }

    private lateinit var textViewResult: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_jnibridge)

        System.loadLibrary("jnibridge")

        textViewResult = findViewById(R.id.textview_result)

        findViewById<Button>(R.id.button_test_jnibridge).setOnClickListener {
            val callback = createNativeCallback()
            val result = callback.onCall("Hello from Java")
            textViewResult.text = "Native callback returned: $result"
        }
    }

    private external fun createNativeCallback(): ICallback
}
