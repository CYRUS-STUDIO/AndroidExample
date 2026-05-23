package com.cyrus.example.webview;

import android.content.Context;
import android.net.Uri;
import android.util.Log;
import android.webkit.WebView;

import androidx.annotation.NonNull;
import androidx.webkit.JavaScriptReplyProxy;
import androidx.webkit.WebMessageCompat;
import androidx.webkit.WebViewCompat;
import androidx.webkit.WebViewFeature;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public class WebShellBridge {

    static {
        System.loadLibrary("webshell_bridge");
    }

    public static void attach(WebView webView) {

        Log.i("WebShell", "supported = " + WebViewFeature.isFeatureSupported(WebViewFeature.WEB_MESSAGE_LISTENER));

        if (!WebViewFeature.isFeatureSupported(WebViewFeature.WEB_MESSAGE_LISTENER)) {
            return;
        }

        Set<String> origins = new HashSet<>();

        origins.add("*");

        WebViewCompat.addWebMessageListener(
                webView,
                "nativeBridge",
                origins,
                new WebViewCompat.WebMessageListener() {

                    @Override
                    public void onPostMessage(
                            @NonNull WebView view,
                            @NonNull WebMessageCompat message,
                            @NonNull Uri sourceOrigin,
                            boolean isMainFrame,
                            @NonNull JavaScriptReplyProxy replyProxy
                    ) {

                        String json = message.getData();

                        nativeHandleMessage(json);

                        replyProxy.postMessage("{\"code\":0}");
                    }
                }
        );
    }

    private static String loadJsFromAssets(Context context, String fileName) {
        StringBuilder sb = new StringBuilder();
        try (InputStream is = context.getAssets().open(fileName);
             BufferedReader reader = new BufferedReader(new InputStreamReader(is))) {

            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
        return sb.toString();
    }

    public static void injectJavascriptShim(WebView webView) {

        String js = loadJsFromAssets(webView.getContext(), "bridge.js");

        webView.evaluateJavascript(js, null);
    }

    public static void registerNamespace(WebView webView, String namespace) {

        if (webView == null || namespace == null || namespace.isEmpty()) {
            return;
        }

        String js = String.format("window.bridgeFactory && bridgeFactory.register('%s');", namespace);

        webView.evaluateJavascript(js, null);
    }

    /**
     * Native层处理
     */
    private static native void nativeHandleMessage(String json);
}