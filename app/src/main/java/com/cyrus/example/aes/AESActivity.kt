package com.cyrus.example.aes

import AESUtils
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlin.random.Random

class AESActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            AESUI()
        }
    }
}

@Composable
fun AESUI() {
    var inputText by remember { mutableStateOf("") }
    var resultText by remember { mutableStateOf("") }
    var showText by remember { mutableStateOf("") }
    var selectedMode by remember { mutableStateOf("CBC") }

    // 在 onCreate 触发一次生成随机字符串的点击事件
    LaunchedEffect(Unit) {
        inputText = generateRandomString()
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Button(onClick = { inputText = generateRandomString() }) {
            Text("生成随机字符串")
        }

        Text(
            text = "输入: $inputText",
            fontSize = 15.sp,
            color = Color.White,
            modifier = Modifier.padding(top = 16.dp)
        )

        Spacer(modifier = Modifier.height(16.dp))

        Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text("编码方式：", fontSize = 18.sp, color = Color.White)
            Spacer(modifier = Modifier.width(8.dp))
            ModeDropdownMenu(selectedMode) { selectedMode = it }
        }

        Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {
            Button(onClick = {
                val input = inputText.toByteArray()

                // 加密
                val data = when (selectedMode) {
                    "ECB" -> NativeAESUtils.aesECBEncode(input)
                    "CTR" -> NativeAESUtils.aesCTREncode(input)
                    else -> NativeAESUtils.aesCBCEncode(input)
                }

                // 将加密后的数据转换为 Hex 字符串显示
                resultText = data.toHexString()

                Log.d("AESActivity", "标准 AES 加密：$inputText -> $resultText")

                showText = "标准 AES 加密：\n\n$inputText\n\n->\n\n$resultText"
            }) {
                Text(
                    "标准 AES 加密",
                    fontSize = 12.sp,
                    color = Color.White,
                )
            }

            Button(onClick = {
                val input = resultText.hexStringToByteArray()

                // 解密
                val data = when (selectedMode) {
                    "ECB" -> NativeAESUtils.aesECBDecode(input)
                    "CTR" -> NativeAESUtils.aesCTRDecode(input)
                    else -> NativeAESUtils.aesCBCDecode(input)
                }

                // 将解密后的数据转换回字符串
                inputText = String(data)

                Log.d("AESActivity", "标准 AES 解密：$resultText -> $inputText")

                showText = "标准 AES 解密：\n\n$resultText\n\n->\n\n$inputText"
            }) {
                Text(
                    "标准 AES 解密",
                    fontSize = 12.sp,
                    color = Color.White,
                )
            }
        }

        Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {

            Button(onClick = {
                val key = "CYRUS STUDIO    "
                val iv = "CYRUS STUDIO    "

                resultText = AESUtils.encrypt(inputText, key, iv, selectedMode)

                Log.d("AESActivity", "标准 Java AES 加密：$inputText -> $resultText")

                showText = "标准 Java AES 加密：\n\n$inputText\n\n->\n\n$resultText"
            }) {
                Text(
                    "标准 Java AES 加密",
                    fontSize = 12.sp,
                    color = Color.White,
                )
            }

            Button(onClick = {
                val key = "CYRUS STUDIO    "
                val iv = "CYRUS STUDIO    "

                inputText = AESUtils.decrypt(resultText, key, iv, selectedMode)

                Log.d("AESActivity", "标准 Java AES 解密：$resultText -> $inputText")

                showText = "标准 Java AES 解密：\n\n$resultText\n\n->\n\n$inputText"
            }) {
                Text(
                    "标准 Java AES 解密",
                    fontSize = 12.sp,
                    color = Color.White,
                )
            }
        }

        Text(
            text = showText,
            fontSize = 15.sp,
            color = Color.White,
            modifier = Modifier.align(Alignment.Start).padding(top = 16.dp)
        )
    }
}

@Composable
fun ModeDropdownMenu(selectedMode: String, onSelectedChange: (String) -> Unit) {
    val modes = listOf("CBC", "ECB", "CTR")
    var expanded by remember { mutableStateOf(false) }

    Box(modifier = Modifier.wrapContentSize(Alignment.TopStart)) {
        Button(onClick = { expanded = true }) {
            Text(selectedMode)
        }
        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }, modifier = Modifier.align(Alignment.TopEnd)) {
            modes.forEachIndexed { index, text ->
                DropdownMenuItem(text = { Text(text) }, onClick = {
                    onSelectedChange(text)
                    expanded = false
                })
            }
        }
    }
}

fun generateRandomString(length: Int = Random.nextInt(5, 16)): String {
    val charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    return (1..length)
        .map { charset[Random.nextInt(charset.length)] }
        .joinToString("")
}


fun getMode(mode: String): Int = when (mode) {
    "CBC" -> 1
    "ECB" -> 2
    "CTR" -> 3
    else -> 1
}