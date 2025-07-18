package com.cyrus.example.retrofit.network

import com.cyrus.example.retrofit.model.User
import retrofit2.http.GET
import retrofit2.http.Path

interface ApiService {
    @GET("users/{id}")
    suspend fun getUser(@Path("id") id: Int): User

    @GET("users/1")
    suspend fun getPlainText(): String
}
