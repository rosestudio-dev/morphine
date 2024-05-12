package morphine.utils

import java.security.SecureRandom

actual fun secureRandom(size: Int): ByteArray {
    val secureRandom = SecureRandom()
    val array = ByteArray(size)

    secureRandom.nextBytes(array)

    return array
}