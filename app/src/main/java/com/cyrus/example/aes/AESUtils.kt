import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

object AESUtils {

    // 将普通字符串转换为 SecretKey
    private fun stringToSecretKey(key: String): SecretKeySpec {
        // 通过 UTF-8 编码将字符串转为字节数组，并确保其长度为 16、24 或 32 字节
        val keyBytes = key.toByteArray(Charsets.UTF_8)
        val validKeyLength = 16 // 默认使用 16 字节
        val keyArray = ByteArray(validKeyLength)

        System.arraycopy(keyBytes, 0, keyArray, 0, Math.min(keyBytes.size, validKeyLength))
        return SecretKeySpec(keyArray, "AES")
    }

    // 将普通字符串转换为 IvParameterSpec
    private fun stringToIV(iv: String): IvParameterSpec {
        // 通过 UTF-8 编码将字符串转为字节数组，确保其长度为 16 字节
        val ivBytes = iv.toByteArray(Charsets.UTF_8)
        val ivArray = ByteArray(16)

        System.arraycopy(ivBytes, 0, ivArray, 0, Math.min(ivBytes.size, 16))
        return IvParameterSpec(ivArray)
    }

    // AES 加密
    fun encrypt(data: String, key: String, iv: String, mode: String): String {
        val secretKey = stringToSecretKey(key)
        val ivSpec = if (mode == "ECB") null else stringToIV(iv)

        val cipher = Cipher.getInstance(getTransformation(mode))
        if (mode == "ECB") {
            cipher.init(Cipher.ENCRYPT_MODE, secretKey)
        } else {
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec)
        }

        val encryptedBytes = cipher.doFinal(data.toByteArray())
        return encryptedBytes.joinToString("") { String.format("%02x", it) }
    }

    // AES 解密
    fun decrypt(encryptedData: String, key: String, iv: String, mode: String): String {
        val secretKey = stringToSecretKey(key)
        val ivSpec = if (mode == "ECB") null else stringToIV(iv)

        val cipher = Cipher.getInstance(getTransformation(mode))
        if (mode == "ECB") {
            cipher.init(Cipher.DECRYPT_MODE, secretKey)
        } else {
            cipher.init(Cipher.DECRYPT_MODE, secretKey, ivSpec)
        }

        val encryptedBytes = encryptedData.chunked(2).map { it.toInt(16).toByte() }.toByteArray()
        val decryptedBytes = cipher.doFinal(encryptedBytes)
        return String(decryptedBytes)
    }

    // 根据模式获取 Cipher Transformation
    private fun getTransformation(mode: String): String {
        return when (mode) {
            "CBC" -> "AES/CBC/PKCS5Padding"
            "ECB" -> "AES/ECB/PKCS5Padding"
            "CTR" -> "AES/CTR/NoPadding"
            else -> throw IllegalArgumentException("Unsupported mode: $mode")
        }
    }
}
