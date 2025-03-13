package com.cyrus.example.crc32

import android.os.Bundle
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
import androidx.compose.material3.DropdownMenu
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
import java.util.zip.CRC32
import kotlin.random.Random

class CRC32Activity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            CRC32Screen()
        }
    }
}

@Composable
fun CRC32Screen() {
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
                    fontSize = 20.sp,
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
                    1 -> CRC32Utils.customTableCRC32(it)
                    2 -> CRC32Utils.modifiedCRC32(it)
                    else -> CRC32Utils.crc32(it)
                }
            } ?: Toast.makeText(context, "请先选择一个字符串", Toast.LENGTH_SHORT).show()
        }) {
            Text("计算 CRC32")
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "CRC32 结果: $result",
            fontSize = 20.sp,
            color = Color.White,
            modifier = Modifier.padding(top = 16.dp)
        )
    }
}

@Composable
fun DropdownMenu(selectedIndex: Int, onSelectedChange: (Int) -> Unit) {
    val options = listOf("标准 CRC32", "自定义 Table CRC32", "魔改版 CRC32")
    var expanded by remember { mutableStateOf(false) }

    Box(modifier = Modifier.wrapContentSize(Alignment.TopStart)) {
        Button(onClick = { expanded = true }) {
            Text(options[selectedIndex])
        }
        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }, modifier = Modifier.align(Alignment.TopEnd)) {
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


fun standardCRC32(input: String): String {
    val crc32 = CRC32()
    crc32.update(input.toByteArray())
    return crc32.value.toString(16)
}

fun customTableCRC32(input: String): String {
    val table = LongArray(256) { i ->
        var crc = i.toLong()
        repeat(8) { crc = if ((crc and 1) != 0L) (crc ushr 1) xor 0xEDB88320 else crc ushr 1 }
        crc
    }
    var crc = 0xFFFFFFFFL
    input.toByteArray().forEach { crc = table[((crc xor it.toLong()) and 0xFF).toInt()] xor (crc ushr 8) }
    return (crc xor 0xFFFFFFFFL).toString(16)
}

fun modifiedCRC32(input: String): String {
    var crc = 0xFFFFFFFFL
    input.toByteArray().forEach { crc = (crc shr 4) xor ((crc and 0xF) * 0x1081) xor it.toLong() }
    return (crc xor 0xFFFFFFFFL).toString(16)
}