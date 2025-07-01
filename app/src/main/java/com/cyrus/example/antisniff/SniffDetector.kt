package com.cyrus.example.antisniff

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities

object SniffDetector {

    /**
     * 检查是否设置了系统代理
     */
    fun checkSystemProxy(): String {
        val httpHost = System.getProperty("http.proxyHost")
        val httpPort = System.getProperty("http.proxyPort")
        val httpsHost = System.getProperty("https.proxyHost")
        val httpsPort = System.getProperty("https.proxyPort")

        return if (!httpHost.isNullOrEmpty() || !httpsHost.isNullOrEmpty()) {
            """
            检测到系统代理:
            HTTP: $httpHost:$httpPort
            HTTPS: $httpsHost:$httpsPort
            """.trimIndent()
        } else {
            "未检测到系统代理"
        }
    }

    /**
     * 检测 VPN 是否活跃
     */
    fun isVpnActive(context: Context): Boolean {
        val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as? ConnectivityManager
            ?: return false
        val networks = connectivityManager.allNetworks
        for (network in networks) {
            val caps = connectivityManager.getNetworkCapabilities(network)
            if (caps != null && caps.hasTransport(NetworkCapabilities.TRANSPORT_VPN)) {
                return true
            }
        }
        return false
    }

    /**
     * 检测是否通过 ADB 设置了 HTTP 代理（常用于抓包）
     */
    fun isAdbProxyEnabled(): Boolean {
        val proxyHost = System.getProperty("http.proxyHost")
        val proxyPort = System.getProperty("http.proxyPort")
        if (proxyHost == "127.0.0.1" && proxyPort == "8888") {
            // 可能为 adb proxy 抓包
            return true
        }
        return false
    }

}
