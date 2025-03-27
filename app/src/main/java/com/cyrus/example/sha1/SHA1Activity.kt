package com.cyrus.example.sha1

import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlin.random.Random

class SHA1Activity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            SHA1Screen()
        }
    }
}

@Composable
fun SHA1Screen() {
    val context = LocalContext.current
    var stringList by remember { mutableStateOf(generateRandomStrings()) }
    var selectedString by remember { mutableStateOf<String?>(null) }
    var selectedMethod by remember { mutableStateOf(0) }
    var result by remember { mutableStateOf("") }

    Column(modifier = Modifier.padding(16.dp)) {
        Button(onClick = {
            stringList = generateRandomStrings()
            selectedString = null
        }) {
            Text("生成随机字符串")
        }

        Spacer(modifier = Modifier.height(8.dp))

        LazyColumn(modifier = Modifier.height(280.dp)) {
            items(stringList) { item ->
                Text(
                    text = item,
                    fontSize = 18.sp,
                    color = Color.White,
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(8.dp)
                        .background(if (selectedString == item) Color.Gray else Color.Transparent)
                        .clickable { selectedString = item }
                        .padding(8.dp)
                )
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text("编码方式：", fontSize = 18.sp, color = Color.White)
            Spacer(modifier = Modifier.width(8.dp))
            DropdownMenu(selectedMethod) { selectedMethod = it }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Button(onClick = {
            selectedString?.let {
                result = when (selectedMethod) {
                    1 -> SHA1Utils.changeSHA1Init(it)
                    2 -> SHA1Utils.changeSHA1Update(it)
                    3 -> SHA1Utils.changeConstant(it)
                    4 -> SHA1Utils.javaSHA1(it)
                    else -> SHA1Utils.sha1(it)
                }
                Log.d("SHA1", "$selectedString -> $result")
            } ?: Toast.makeText(context, "请先选择一个字符串", Toast.LENGTH_SHORT).show()
        }) {
            Text("计算 SHA1")
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "SHA1 结果: $result",
            fontSize = 18.sp,
            color = Color.White,
            modifier = Modifier.padding(top = 16.dp)
        )
    }
}

@Composable
fun DropdownMenu(selectedIndex: Int, onSelectedChange: (Int) -> Unit) {
    val options = listOf("标准SHA1", "SHA1Init变形", "SHA1Update变形", "宏常量变形", "标准 Java SHA1")
    var expanded by remember { mutableStateOf(false) }

    Box(modifier = Modifier.wrapContentSize(Alignment.TopStart)) {
        Button(onClick = { expanded = true }) {
            Text(options[selectedIndex])
        }
        androidx.compose.material3.DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }, modifier = Modifier.align(Alignment.TopEnd)) {
            options.forEachIndexed { index, text ->
                DropdownMenuItem(text = { Text(text) }, onClick = {
                    onSelectedChange(index)
                    expanded = false
                })
            }
        }
    }
}


fun generateRandomStrings(count: Int = 5, minLength: Int = 5, maxLength: Int = 15): List<String> {
    val charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    return List(count) {
        val length = Random.nextInt(minLength, maxLength + 1) // 生成 [minLength, maxLength] 范围内的随机长度
        (1..length)
            .map { charset.random() }
            .joinToString("")
    }
}