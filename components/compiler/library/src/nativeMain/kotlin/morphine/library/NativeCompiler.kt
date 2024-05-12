package morphine.library

import compiler.library.BuildConfig
import kotlinx.cinterop.CPointer
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.allocArrayOf
import kotlinx.cinterop.free
import kotlinx.cinterop.nativeHeap
import morphine.Compiler

@OptIn(ExperimentalForeignApi::class)
class NativeCompiler {

    private var result: Compiler.Result<List<Byte>>? = null

    private val heap = mutableListOf<CPointer<*>>()

    fun compile(text: String, optimize: Boolean): Boolean {
        result = Compiler(
            text = text,
            optimize = optimize
        ).compileBytes()

        return result is Compiler.Result.Error
    }

    fun getError() = when (val result = result) {
        is Compiler.Result.Error -> result.message.allocate()
        is Compiler.Result.Success -> null
        null -> null
    }

    fun getBytecodeSize() = when (val result = result) {
        is Compiler.Result.Error -> 0
        is Compiler.Result.Success -> result.data.size
        null -> 0
    }

    fun getBytecode() = when (val result = result) {
        is Compiler.Result.Error -> null
        is Compiler.Result.Success -> result.data.allocate()
        null -> null
    }

    fun versionCode() = BuildConfig.versionCode.toInt()

    fun version() = BuildConfig.version.allocate()

    fun release() {
        heap.forEach(nativeHeap::free)
        heap.clear()
    }

    private fun String.allocate() =
        nativeHeap.allocArrayOf(this.encodeToByteArray() + byteArrayOf(0)).recognize()

    private fun List<Byte>.allocate() =
        nativeHeap.allocArrayOf(this.toByteArray()).recognize()

    private fun <T, P : CPointer<T>> P.recognize(): P = this.also(heap::add)
}