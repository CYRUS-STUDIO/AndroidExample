import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.*
import okhttp3.logging.HttpLoggingInterceptor
import okhttp3.Headers.Companion.toHeaders
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.asRequestBody
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.File
import java.io.IOException
import java.util.concurrent.TimeUnit

object NetworkClient {

    private var loggingEnabled = false
    private val JSON = "application/json; charset=utf-8".toMediaType()
    private val FORM = "application/x-www-form-urlencoded".toMediaType()

    private var globalHeaders: MutableMap<String, String> = mutableMapOf()

    private val client: OkHttpClient by lazy {
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

        builder.build()
    }

    // 启用日志拦截器（必须在首次请求前调用）
    fun enableLogging() {
        loggingEnabled = true
    }

    // 设置全局请求头（如 token）
    fun setGlobalHeader(key: String, value: String) {
        globalHeaders[key] = value
    }

    fun clearGlobalHeaders() {
        globalHeaders.clear()
    }

    private fun buildHeaders(custom: Map<String, String>? = null): Headers {
        val all = globalHeaders.toMutableMap()
        custom?.let { all.putAll(it) }
        return all.toHeaders()
    }

    // ========= 协程版本（推荐） =========

    suspend fun getSuspend(url: String, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val request = Request.Builder()
                .url(url)
                .headers(buildHeaders(headers))
                .get()
                .build()

            client.newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

    suspend fun postJsonSuspend(url: String, json: String, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val body = json.toRequestBody(JSON)
            val request = Request.Builder()
                .url(url)
                .headers(buildHeaders(headers))
                .post(body)
                .build()

            client.newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

    suspend fun postFormSuspend(url: String, formData: Map<String, String>, headers: Map<String, String>? = null): String =
        withContext(Dispatchers.IO) {
            val formBody = FormBody.Builder().apply {
                formData.forEach { (k, v) -> add(k, v) }
            }.build()

            val request = Request.Builder()
                .url(url)
                .headers(buildHeaders(headers))
                .post(formBody)
                .build()

            client.newCall(request).execute().use {
                if (!it.isSuccessful) throw IOException("Unexpected code $it")
                it.body?.string() ?: ""
            }
        }

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

        val request = Request.Builder()
            .url(url)
            .headers(buildHeaders(headers))
            .post(multipartBody)
            .build()

        client.newCall(request).execute().use {
            if (!it.isSuccessful) throw IOException("Upload failed: $it")
            it.body?.string() ?: ""
        }
    }

    // ========= 原始 Callback 异步版本（兼容旧代码） =========

    fun get(url: String, headers: Map<String, String>? = null, callback: Callback) {
        val request = Request.Builder()
            .url(url)
            .headers(buildHeaders(headers))
            .get()
            .build()

        client.newCall(request).enqueue(callback)
    }

    fun postJson(url: String, json: String, headers: Map<String, String>? = null, callback: Callback) {
        val body = json.toRequestBody(JSON)
        val request = Request.Builder()
            .url(url)
            .headers(buildHeaders(headers))
            .post(body)
            .build()

        client.newCall(request).enqueue(callback)
    }

    fun postForm(url: String, formData: Map<String, String>, headers: Map<String, String>? = null, callback: Callback) {
        val formBody = FormBody.Builder().apply {
            formData.forEach { (k, v) -> add(k, v) }
        }.build()

        val request = Request.Builder()
            .url(url)
            .headers(buildHeaders(headers))
            .post(formBody)
            .build()

        client.newCall(request).enqueue(callback)
    }
}
