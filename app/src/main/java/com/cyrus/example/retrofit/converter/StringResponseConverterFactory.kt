package com.cyrus.example.retrofit.converter

import okhttp3.ResponseBody
import retrofit2.Converter
import retrofit2.Retrofit
import java.lang.reflect.Type

class StringResponseConverterFactory : Converter.Factory() {

    override fun responseBodyConverter(
        type: Type, annotations: Array<Annotation>, retrofit: Retrofit
    ): Converter<ResponseBody, *>? {
        // 仅处理 String 类型的响应
        return if (type == String::class.java) {
            Converter<ResponseBody, String> { body -> body.string() }
        } else {
            null
        }
    }
}