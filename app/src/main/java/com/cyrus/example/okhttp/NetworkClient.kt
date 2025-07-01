import android.content.Context
import okhttp3.*
import okhttp3.logging.HttpLoggingInterceptor
import okhttp3.Headers.Companion.toHeaders
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.asRequestBody
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.File
import java.io.IOException
import java.io.InputStream
import java.net.Proxy
import java.security.KeyStore
import java.security.cert.CertificateFactory
import java.security.cert.X509Certificate
import java.util.concurrent.TimeUnit
import javax.net.ssl.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

object NetworkClient {

    private var loggingEnabled = false
    private var noProxyEnabled = false
    private var sslPinnedClient: OkHttpClient? = null

    private val JSON = "application/json; charset=utf-8".toMediaType()
    private val FORM = "application/x-www-form-urlencoded".toMediaType()

    private var globalHeaders: MutableMap<String, String> = mutableMapOf()

    private val defaultClient: OkHttpClient by lazy {
        buildClient()
    }

    // 构建默认 OkHttpClient（支持超时、日志、禁用代理）
    private fun buildClient(): OkHttpClient {
        val builder = OkHttpClient.Builder()
            .connectTimeout(15, TimeUnit.SECONDS)
            .readTimeout(15, TimeUnit.SECONDS)
            .writeTimeout(15, TimeUnit.SECONDS)

        if (loggingEnabled) {
            val logging = HttpLoggingInterceptor().apply {
                level = HttpLoggingInterceptor.Level.BODY
            }
            builder.addInterceptor(logging)
        }

        if (noProxyEnabled) {
            builder.proxy(Proxy.NO_PROXY)
        }

        return builder.build()
    }

    // 启用 OkHttp 日志拦截器
    fun enableLogging() {
        loggingEnabled = true
    }

    // 启用无代理模式（防止抓包）
    fun enableNoProxy() {
        noProxyEnabled = true
    }

    // 设置全局请求头
    fun setGlobalHeader(key: String, value: String) {
        globalHeaders[key] = value
    }

    // 清空全局请求头
    fun clearGlobalHeaders() {
        globalHeaders.clear()
    }

    // 构建 Headers 对象（包含全局和自定义）
    private fun buildHeaders(custom: Map<String, String>? = null): Headers {
        val all = globalHeaders.toMutableMap()
        custom?.let { all.putAll(it) }
        return all.toHeaders()
    }

    // 获取当前使用的 OkHttpClient（优先使用 Pinning 客户端）
    private fun getClient(): OkHttpClient = sslPinnedClient ?: defaultClient

    // 启用证书级别 Pinning（证书完全匹配）
    fun enableCertificatePinning(context: Context, assetCertFileName: String) {
        val certInput: InputStream = context.assets.open(assetCertFileName)
        val certificateFactory = CertificateFactory.getInstance("X.509")
        val certificate = certificateFactory.generateCertificate(certInput) as X509Certificate

        val keyStore = KeyStore.getInstance(KeyStore.getDefaultType()).apply {
            load(null, null)
            setCertificateEntry("ca", certificate)
        }

        val trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm()).apply {
            init(keyStore)
        }

        val sslContext = SSLContext.getInstance("TLS").apply {
            init(null, trustManagerFactory.trustManagers, null)
        }

        sslPinnedClient = buildClient().newBuilder()
            .sslSocketFactory(sslContext.socketFactory, trustManagerFactory.trustManagers[0] as X509TrustManager)
            .build()
    }

    // 启用公钥级 Pinning（base64 公钥）
    fun enablePublicKeyPinning(domain: String, base64PublicKey: String) {
        val pinner = CertificatePinner.Builder()
            .add(domain, "sha256/$base64PublicKey")
            .build()
        sslPinnedClient = buildClient().newBuilder()
            .certificatePinner(pinner)
            .build()
    }

    // 启用 SPKI Hash Pinning（推荐方式）
    fun enableSpkiSha256Pinning(domain: String, base64SpkiHash: String) {
        val pinner = CertificatePinner.Builder()
            .add(domain, "sha256/$base64SpkiHash")
            .build()
        sslPinnedClient = buildClient().newBuilder()
            .certificatePinner(pinner)
            .build()
    }

    /**
     * 为多个域名启用 SPKI Hash 形式的 SSL Pinning。
     *
     * @param pins 一个映射，每个键为域名（如 "example.com"），对应值为该域名允许的 SPKI 哈希列表（Base64 编码，带或不带前缀 "sha256/"）。
     * 示例：
     * enableMultiDomainPinning(
     *     mapOf(
     *         "api.example.com" to listOf("Pz89eRE/Pz84Yj8/NgE/P09gP3M/DQo="),
     *         "cdn.example.org" to listOf("ABCDEF123456...", "ZYXWV09876...")
     *     )
     * )
     */
    fun enableMultiDomainSpkiSha256Pinning(pins: Map<String, List<String>>) {
        val builder = CertificatePinner.Builder()
        for ((host, hashes) in pins) {
            hashes.forEach { hash ->
                builder.add(host, "sha256/$hash")
            }
        }
        sslPinnedClient = buildClient().newBuilder()
            .certificatePinner(builder.build())
            .build()
    }


    // suspend GET 请求
    suspend fun getSuspend(url: String, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val request = Request.Builder().url(url).headers(buildHeaders(headers)).get().build()
            getClient().newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

    // suspend POST JSON 请求
    suspend fun postJsonSuspend(url: String, json: String, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val body = json.toRequestBody(JSON)
            val request = Request.Builder().url(url).headers(buildHeaders(headers)).post(body).build()
            getClient().newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

    // suspend POST 表单请求
    suspend fun postFormSuspend(url: String, formData: Map<String, String>, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val formBody = FormBody.Builder().apply {
                formData.forEach { (k, v) -> add(k, v) }
            }.build()
            val request = Request.Builder().url(url).headers(buildHeaders(headers)).post(formBody).build()
            getClient().newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

    // suspend 上传文件请求（支持额外表单字段）
    suspend fun uploadFileSuspend(
        url: String,
        file: File,
        fileField: String = "file",
        extraFormData: Map<String, String>? = null,
        headers: Map<String, String>? = null
    ): String = withContext(Dispatchers.IO) {
        val fileBody = file.asRequestBody("application/octet-stream".toMediaType())
        val multipartBody = MultipartBody.Builder().setType(MultipartBody.FORM).apply {
            addFormDataPart(fileField, file.name, fileBody)
            extraFormData?.forEach { (k, v) -> addFormDataPart(k, v) }
        }.build()
        val request = Request.Builder().url(url).headers(buildHeaders(headers)).post(multipartBody).build()
        getClient().newCall(request).execute().use {
            if (!it.isSuccessful) throw IOException("Upload failed: $it")
            it.body?.string() ?: ""
        }
    }

    // 异步 GET 请求
    fun get(url: String, headers: Map<String, String>? = null, callback: Callback) {
        val request = Request.Builder().url(url).headers(buildHeaders(headers)).get().build()
        getClient().newCall(request).enqueue(callback)
    }

    // 异步 POST JSON 请求
    fun postJson(url: String, json: String, headers: Map<String, String>? = null, callback: Callback) {
        val body = json.toRequestBody(JSON)
        val request = Request.Builder().url(url).headers(buildHeaders(headers)).post(body).build()
        getClient().newCall(request).enqueue(callback)
    }

    // 异步 POST 表单请求
    fun postForm(url: String, formData: Map<String, String>, headers: Map<String, String>? = null, callback: Callback) {
        val formBody = FormBody.Builder().apply {
            formData.forEach { (k, v) -> add(k, v) }
        }.build()
        val request = Request.Builder().url(url).headers(buildHeaders(headers)).post(formBody).build()
        getClient().newCall(request).enqueue(callback)
    }
}
