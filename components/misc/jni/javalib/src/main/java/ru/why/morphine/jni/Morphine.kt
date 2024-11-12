package ru.why.morphine.jni

import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.InputStream
import java.io.OutputStream

class Morphine(
    private val settings: Settings = Settings(),
    private val bridge: Bridge = Bridge(),
    private val receiver: Receiver? = null
) {
    companion object {
        init {
            System.loadLibrary("morphinejni")
        }
    }

    private val syncVm = Object()
    private val syncQueue = Object()

    @Volatile
    private var interrupt = false
    private val queue = ArrayDeque<Value>()

    fun compile(text: String): ByteArray {
        synchronized(syncVm) {
            val outputStream = ByteArrayOutputStream()
            if (compiler(outputStream, text)) {
                throw RuntimeException()
            }

            return outputStream.toByteArray()
        }
    }

    fun execute(text: String) {
        synchronized(syncVm) {
            val outputStream = ByteArrayOutputStream()
            if (compiler(outputStream, text)) {
                throw RuntimeException()
            }

            execute(outputStream.toByteArray())
        }
    }

    fun execute(binary: ByteArray) {
        synchronized(syncVm) {
            interrupt = false
            clearQueue()
            val inputStream = ByteArrayInputStream(binary)
            if (interpreter(inputStream)) {
                throw RuntimeException()
            }
        }
    }

    fun interrupt() {
        interrupt = true
    }

    fun send(value: Value) {
        synchronized(syncQueue) {
            queue.addLast(value)
        }
    }

    private fun clearQueue() {
        synchronized(syncQueue) {
            queue.clear()
        }
    }

    private fun receive(): Value {
        synchronized(syncQueue) {
            return queue.removeFirstOrNull() ?: Value.Primitive.MLNil
        }
    }

    private fun waitReceive(): Value {
        while (true) {
            synchronized(syncQueue) {
                if (queue.isNotEmpty()) {
                    return queue.removeFirst()
                }
            }
        }
    }

    private external fun compiler(out: OutputStream, text: String): Boolean
    private external fun interpreter(input: InputStream): Boolean

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

    sealed interface Value {
        sealed interface Primitive : Value {
            data object MLNil : Primitive
            data class MLInteger(@JvmField val value: Long) : Primitive
            data class MLDecimal(@JvmField val value: Double) : Primitive
            data class MLBoolean(@JvmField val value: Boolean) : Primitive
            data class MLString(@JvmField val value: String) : Primitive
        }

        data class MLVector(@JvmField val value: List<Primitive>) : Value
        data class MLTable(@JvmField val value: Map<Primitive, Primitive>) : Value
    }

    fun interface Receiver {
        fun receive(value: Value)
    }
}
