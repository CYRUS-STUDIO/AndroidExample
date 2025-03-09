package com.cyrus.example.base64

import android.os.Bundle
import android.util.Base64
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

class Base64Activity : ComponentActivity() {

    // 加载 native 库
    init {
        System.loadLibrary("base64")
    }

    // 标准 Base64（C++）
    external fun nativeBase64Encode(data: ByteArray): String
    external fun nativeBase64Decode(input: String): ByteArray

    // 自定义 Base64 编码和解码
    external fun customBase64Encode(data: ByteArray): String
    external fun customBase64Decode(encoded: String): ByteArray

    // 动态 Base64 编码和解码
    external fun dynamicBase64Encode(input: ByteArray): String
    external fun dynamicBase64Decode(input: String, originalLength: Int): ByteArray

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            Base64App(::nativeBase64Encode, ::nativeBase64Decode, ::customBase64Encode, ::customBase64Decode, ::dynamicBase64Encode, ::dynamicBase64Decode)
        }
    }
}


@Composable
fun Base64App(encode: (ByteArray) -> String,
              decode: (String) -> ByteArray,
              customEncode: (ByteArray) -> String,  // 自定义Base64编码
              customDecode: (String) -> ByteArray,   // 自定义Base64解码
              dynamicBase64Encode: (ByteArray) -> String,  // 动态Base64编码
              dynamicBase64Decode: (String, Int) -> ByteArray   // 动态Base64解码
              ) {
    var stringList by remember { mutableStateOf(listOf<String>()) }
    var selectedString by remember { mutableStateOf<String?>(null) }

    // 字符串长度记录，初始化为 0
    var stringLength by remember { mutableStateOf(0) }

    Column(modifier = Modifier.padding(16.dp)) {
        Button(
            onClick = { stringList = generateRandomStrings(5) },
            modifier = Modifier.fillMaxWidth() // 按钮宽度填满屏幕
        ) {
            Text("生成随机字符串", style = MaterialTheme.typography.bodyMedium)
        }

        Spacer(modifier = Modifier.height(10.dp))

        LazyColumn(modifier = Modifier.weight(1f)) {
            items(stringList) { item ->
                StringListItem(
                    text = item,
                    isSelected = item == selectedString,
                    onClick = { selectedString = item }
                )
            }
        }

        selectedString?.let { selected ->
            Spacer(modifier = Modifier.height(10.dp))

            Box(
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(
                    text = "选中: $selected",
                    style = MaterialTheme.typography.bodyMedium,
                    color = Color.White, // 设置文字颜色为白色
                    modifier = Modifier.align(Alignment.Center) // 文字居中
                )
            }

            Spacer(modifier = Modifier.height(10.dp))

            // Java 编码和解码按钮排在上面
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp) // 增加按钮之间的间距
            ) {
                Button(
                    onClick = {
                        val javaEncoded = Base64.encodeToString(selected.toByteArray(), Base64.DEFAULT)
                        Log.d("Base64", "标准 Base64 编码（Java）: $javaEncoded")
                        selectedString = javaEncoded
                    },
                    modifier = Modifier.weight(1f) // 按钮宽度自适应
                ) {
                    Text("标准 Base64 编码（Java）", style = MaterialTheme.typography.bodySmall) // 调小文字大小
                }

                Button(
                    onClick = {
                        try {
                            val javaDecodedString = String(Base64.decode(selected, Base64.DEFAULT))
                            Log.d("Base64", "标准 Base64 解码（Java）: $javaDecodedString")
                            selectedString = javaDecodedString
                        } catch (e: Exception) {
                            Log.e("Base64", "解码失败: ${e.message}")
                            selectedString = "解码失败"
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("标准 Base64 解码（Java）", style = MaterialTheme.typography.bodySmall) // 调小文字大小
                }
            }

            Spacer(modifier = Modifier.height(10.dp))

            // C++ 编码和解码按钮
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Button(
                    onClick = {
                        val encoded = encode(selected.toByteArray())
                        Log.d("Base64", "标准 Base64 编码（C++）: $encoded")
                        selectedString = encoded
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("标准 Base64 编码（C++）", style = MaterialTheme.typography.bodySmall) // 调小文字大小
                }

                Button(
                    onClick = {
                        try {
                            val decodedString = String(decode(selected))
                            Log.d("Base64", "标准 Base64 解码（C++）: $decodedString")
                            selectedString = decodedString
                        } catch (e: Exception) {
                            Log.e("Base64", "解码失败: ${e.message}")
                            selectedString = "解码失败"
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("标准 Base64 解码（C++）", style = MaterialTheme.typography.bodySmall) // 调小文字大小
                }
            }

            Spacer(modifier = Modifier.height(10.dp))

            // 自定义 Base64 编码和解码按钮
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Button(
                    onClick = {
                        val encoded = customEncode(selected.toByteArray())
                        Log.d("Base64", "自定义Base64 编码后的字符串: $encoded")
                        selectedString = encoded
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Base64 编码（自定义码表）", style = MaterialTheme.typography.bodySmall)
                }

                Button(
                    onClick = {
                        try {
                            val decodedString = String(customDecode(selected))
                            Log.d("Base64", "自定义Base64 解码后的字符串: $decodedString")
                            selectedString = decodedString
                        } catch (e: Exception) {
                            Log.e("Base64", "解码失败: ${e.message}")
                            selectedString = "解码失败"
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Base64 解码（自定义码表）", style = MaterialTheme.typography.bodySmall)
                }
            }

            Spacer(modifier = Modifier.height(10.dp))

            // 动态码表 Base64 按钮
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                Button(
                    onClick = {
                        val data = selected.toByteArray()
                        stringLength = data.size
                        val encoded = dynamicBase64Encode(data)
                        Log.d("Base64", "动态码表编码后的字符串: $encoded，字符串长度：$stringLength")
                        selectedString = encoded
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Base64 编码（动态码表）", style = MaterialTheme.typography.bodySmall)
                }

                Button(
                    onClick = {
                        try {
                            val decodedBytes = dynamicBase64Decode(selected, stringLength)
                            val decodedString = String(decodedBytes)
                            Log.d("Base64", "动态码表解码后的字符串: $decodedString，字符串长度：$stringLength")
                            selectedString = decodedString
                        } catch (e: Exception) {
                            Log.e("Base64", "解码失败: ${e.message}")
                            selectedString = "解码失败"
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Base64 解码（动态码表）", style = MaterialTheme.typography.bodySmall)
                }
            }
        }
    }
}


@Composable
fun StringListItem(text: String, isSelected: Boolean, onClick: () -> Unit) {
    Card(
        colors = CardDefaults.cardColors(
            containerColor = if (isSelected) Color(0xFFBBDEFB) else Color.White
        ),
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(8.dp))
            .clickable { onClick() }
            .padding(horizontal = 16.dp, vertical = 6.dp),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
    ) {
        Text(
            text = text,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.padding(horizontal = 6.dp) // 让文字与 Card 保持间距
        )
    }
}




fun generateRandomStrings(count: Int): List<String> {
    val chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    return List(count) {
        (1..8).map { chars.random() }.joinToString("")
    }
}
