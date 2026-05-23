package com.cyrus.example.webview;

import android.os.Bundle;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.appcompat.app.AppCompatActivity;

public class WebShellActivity extends AppCompatActivity {

    private WebView webView;

    @Override
    protected void onCreate(
            Bundle savedInstanceState
    ) {

        super.onCreate(savedInstanceState);

        webView = new WebView(this);

        setContentView(webView);

        WebSettings settings =
                webView.getSettings();

        settings.setJavaScriptEnabled(true);

        // 先注册 MessageListener
        WebShellBridge.attach(webView);

        webView.setWebViewClient(
                new WebViewClient() {

                    @Override
                    public void onPageFinished(
                            WebView view,
                            String url
                    ) {

                        // 页面加载完成后再注入兼容层
                        WebShellBridge.injectJavascriptShim(view);

                        // 3 秒后动态注册 jsBridge
                        view.postDelayed(
                                new Runnable() {
                                    @Override
                                    public void run() {
                                        WebShellBridge.registerNamespace(view, "jsBridge");
                                    }
                                },
                                3000
                        );
                    }
                }
        );

        // 最后加载页面
        webView.loadUrl("file:///android_asset/webshell_demo.html");
    }
}
