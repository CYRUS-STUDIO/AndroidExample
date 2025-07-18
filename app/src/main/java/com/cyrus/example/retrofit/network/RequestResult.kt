package com.cyrus.example.retrofit.network

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class RequestResult<T>(
    private val block: suspend () -> T
) {
    private var success: ((T) -> Unit)? = null
    private var failure: ((Throwable) -> Unit)? = null

    fun onSuccess(block: (T) -> Unit): RequestResult<T> {
        success = block
        return this
    }

    fun onFailure(block: (Throwable) -> Unit): RequestResult<T> {
        failure = block
        return this
    }

    fun launch(scope: CoroutineScope) {
        scope.launch(Dispatchers.IO) {
            try {
                val result = block()
                withContext(Dispatchers.Main) {
                    success?.invoke(result)
                }
            } catch (e: Throwable) {
                withContext(Dispatchers.Main) {
                    failure?.invoke(e)
                }
            }
        }
    }
}

fun <T> request(block: suspend () -> T): RequestResult<T> {
    return RequestResult(block)
}
