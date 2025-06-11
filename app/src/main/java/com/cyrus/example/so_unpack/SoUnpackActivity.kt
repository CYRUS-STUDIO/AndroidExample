package com.cyrus.example.so_unpack

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

/**
 * so脱壳
 */
class SoUnpackActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            SoShellScreen()
        }
    }
}

@Composable
fun SoShellScreen() {
    var mapsList by remember { mutableStateOf<List<MemoryMapEntry>>(emptyList()) }
    var logText by remember { mutableStateOf("") }
    var modulesList by remember { mutableStateOf<List<String>>(emptyList()) }

    Column(modifier = Modifier
        .fillMaxSize()
        .padding(16.dp)) {

        Button(onClick = {
            mapsList = MapsReader.readMaps()
        }) {
            Text("读取 /proc/self/maps")
        }

        Spacer(modifier = Modifier.height(16.dp))

        Button(onClick = {
            val logs = StringBuilder()
            val maps = MapsReader.readMaps()
            val soEntries = maps.filter { it.path?.contains(".so") == true }

            for (entry in soEntries) {
                val result = MemoryProtector.protect(
                    entry.startAddress,
                    entry.endAddress - entry.startAddress,
                    MemoryProtector.PROT_READ or
                            MemoryProtector.PROT_WRITE or
                            MemoryProtector.PROT_EXEC
                )
                logs.append(
                    if (result == 0)
                        "✔ RWX set: ${entry.path} [0x${entry.startAddress.toString(16)} - 0x${entry.endAddress.toString(16)}]\n"
                    else
                        "❌ Failed: ${entry.path} [0x${entry.startAddress.toString(16)} - 0x${entry.endAddress.toString(16)}]\n"
                )
            }

            logText = logs.toString()
        }) {
            Text("Memory.protect")
        }

        Spacer(modifier = Modifier.height(16.dp))

        Button(onClick = {
            // 调用 SoInfoUtils.enumerateSoModules()
            val modules = SoInfoUtils.enumerateSoModules()
            modulesList = modules?.toList() ?: emptyList()
        }) {
            Text("enumerateModules")
        }

        Spacer(modifier = Modifier.height(16.dp))

        LazyColumn(modifier = Modifier.fillMaxSize()) {
            items(mapsList.size) { index ->
                val entry = mapsList[index]
                Text(
                    text = "0x${entry.startAddress.toString(16)}-0x${entry.endAddress.toString(16)} " +
                            "${entry.permissions} ${entry.path ?: ""}",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.White
                )
                Spacer(modifier = Modifier.height(4.dp))
            }

            if (modulesList.isNotEmpty()) {
                item {
                    Spacer(modifier = Modifier.height(16.dp))
                    Text(
                        text = "Enumerated Modules:",
                        style = MaterialTheme.typography.bodyMedium,
                        color = Color.Cyan
                    )
                }

                items(modulesList.size) { index ->
                    Text(
                        text = modulesList[index],
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.Green
                    )
                    Spacer(modifier = Modifier.height(2.dp))
                }
            }

            item {
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    text = logText,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Yellow
                )
            }
        }
    }
}
