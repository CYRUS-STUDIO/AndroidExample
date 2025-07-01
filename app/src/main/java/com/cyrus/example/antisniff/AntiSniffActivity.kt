package com.cyrus.example.antisniff

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.Button
import androidx.compose.material.Tab
import androidx.compose.material.TabRow
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.cyrus.example.okhttp.OkHttpDemoScreen
import com.google.accompanist.pager.ExperimentalPagerApi
import com.google.accompanist.pager.HorizontalPager
import com.google.accompanist.pager.rememberPagerState
import kotlinx.coroutines.launch

class AntiSniffActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 启用日志拦截器
        NetworkClient.enableLogging()

        // 设置全局请求头（例如 Token）
        NetworkClient.setGlobalHeader("Authorization", "Bearer your_token")

        setContent {
            AntiSniffScreen()
        }
    }
}

@OptIn(ExperimentalPagerApi::class)
@Composable
fun AntiSniffScreen() {
    val tabTitles = listOf("反抓包", "OkHttp")
    val pagerState = rememberPagerState()
    val coroutineScope = rememberCoroutineScope()

    Column {
        TabRow(selectedTabIndex = pagerState.currentPage) {
            tabTitles.forEachIndexed { index, title ->
                Tab(
                    selected = pagerState.currentPage == index,
                    onClick = { coroutineScope.launch { pagerState.animateScrollToPage(index) } },
                    text = { Text(title) }
                )
            }
        }

        HorizontalPager(
            count = tabTitles.size,
            state = pagerState,
            modifier = Modifier.weight(1f)
        ) { page ->
            when (page) {
                0 -> AntiSniffTabContent()
                1 -> OkHttpDemoScreen()
            }
        }
    }
}

@Composable
fun AntiSniffTabContent() {
    val context = LocalContext.current
    var resultText by remember { mutableStateOf("操作结果将显示在这里") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        Button(onClick = {
            // 启用无代理（防抓包，禁用系统代理）
            NetworkClient.enableNoProxy()
            resultText = "已执行 No Proxy（防抓包，禁用系统代理）"
        }) {
            Text("No Proxy")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {
            NetworkClient.enableSpkiSha256Pinning("httpbin.org", "IFG+z/oQKXfpUYOHgWHy5axgkT9B01XSxwb2AHDyN34=")
            resultText = "已执行 SSL Pinning"
        }) {
            Text("SSL Pinning")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {
            resultText = SniffDetector.checkSystemProxy()
        }) {
            Text("检查系统代理")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {
            resultText = if (SniffDetector.isVpnActive(context)) "检测到 VPN 正在使用" else "未检测到 VPN"
        }) {
            Text("检测 VPN")
        }

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {
            resultText = if (SniffDetector.isAdbProxyEnabled()) "检测到 ADB 代理已开启" else "ADB 代理未开启"
        }) {
            Text("检测 ADB 代理")
        }

        Spacer(modifier = Modifier.height(16.dp))

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = resultText,
            color = Color.White,
            modifier = Modifier
                .fillMaxWidth()
                .padding(8.dp)
        )
    }
}
