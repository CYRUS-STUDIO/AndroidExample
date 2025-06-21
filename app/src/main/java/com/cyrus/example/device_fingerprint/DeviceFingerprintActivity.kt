package com.cyrus.example.device_fingerprint

import android.annotation.SuppressLint
import android.os.Bundle
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

/**
 * 设备指纹
 */
class DeviceFingerprintActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            DeviceFingerprintScreen()
        }
    }

    @Composable
    fun DeviceFingerprintScreen() {
        // 控制输出显示文本
        var output by remember { mutableStateOf("") }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Button(
                onClick = {
                    val androidId = getAndroidId()
                    output = "Android ID: $androidId"
                },
            ) {
                Text("Android ID", color = Color.White)
            }

            Spacer(modifier = Modifier.height(16.dp))

            Text(text = output, color = Color.White)
        }
    }

    @SuppressLint("HardwareIds")
    private fun getAndroidId(): String {
        return Settings.Secure.getString(contentResolver, Settings.Secure.ANDROID_ID) ?: "Unavailable"
    }
}
