package ru.why.morphine.jni

import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.InputStream
import java.io.OutputStream

class Morphine(
    private val settings: Settings = Settings(),
    private val bridge: Bridge = Bridge(),
    private val callable: Callable? = null
) {
    companion object {
        init {
            System.loadLibrary("morphinejni")
        }
    }

    fun compile(text: String): ByteArray {
        val outputStream = ByteArrayOutputStream()
        compiler(outputStream, text)
        return outputStream.toByteArray()
    }

    fun execute(text: String) {
        val outputStream = ByteArrayOutputStream()
        compiler(outputStream, text)
        execute(outputStream.toByteArray())
    }

    fun execute(binary: ByteArray) {
        interpreter(ByteArrayInputStream(binary))
    }

    private external fun compiler(out: OutputStream, text: String)
    private external fun interpreter(input: InputStream)

    data class Settings(
        @JvmField val gcLimit: Int = 8 * 1024 * 1024,
        @JvmField val gcThreshold: Int = 16384,
        @JvmField val gcGrow: Int = 150,
        @JvmField val gcDeal: Int = 200,
        @JvmField val gcPause: Int = 13,
        @JvmField val gcCacheCallinfo: Int = 64,
        @JvmField val coroutinesStackLimit: Int = 65536
    )

    data class Bridge(
        @JvmField val inputStream: InputStream = System.`in`,
        @JvmField val outputStream: OutputStream = System.out,
        @JvmField val errorStream: OutputStream = System.err
    )

    fun interface Callable {
        fun call(method: String, args: Array<MorphineValue>): MorphineValue
    }
}
