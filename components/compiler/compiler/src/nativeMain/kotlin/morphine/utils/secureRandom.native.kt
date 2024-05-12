package morphine.utils

import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.usePinned
import platform.posix.O_RDONLY
import platform.posix.close
import platform.posix.errno
import platform.posix.open
import platform.posix.read

@OptIn(ExperimentalForeignApi::class)
actual fun secureRandom(size: Int): ByteArray {
    val array = ByteArray(size)
    val file = open("/dev/urandom", O_RDONLY)

    check(file != -1) { "Failed to access /dev/urandom: $errno" }

    try {
        array.usePinned { pin ->
            read(file, pin.addressOf(0), size.toULong())
        }
    } finally {
        close(file)
    }

    return array
}