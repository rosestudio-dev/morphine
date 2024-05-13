package morphine.utils

import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.alloc
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
import platform.posix.gettimeofday
import platform.posix.timeval

@OptIn(ExperimentalForeignApi::class)
actual fun millis() = memScoped {
    val time = alloc<timeval>()
    gettimeofday(time.ptr, null)

    time.tv_sec * 1000 + time.tv_usec / 1000
}