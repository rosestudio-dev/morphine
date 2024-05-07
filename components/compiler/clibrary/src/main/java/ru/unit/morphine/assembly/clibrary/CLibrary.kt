package ru.unit.morphine.assembly.clibrary

import org.graalvm.nativeimage.IsolateThread
import org.graalvm.nativeimage.c.function.CEntryPoint
import org.graalvm.nativeimage.c.type.CCharPointer
import org.graalvm.nativeimage.c.type.CTypeConversion
import ru.unit.morphine.assembly.clibrary.clibrary.BuildConfig
import ru.unit.morphine.assembly.compiler.MorphineCompiler

object CLibrary {

    private var assembleResult: Result? = null

    @JvmStatic
    @CEntryPoint(name = "libcompiler_assemble")
    private fun compile(
        thread: IsolateThread,
        text: CCharPointer,
        optimize: Boolean
    ): Boolean {
        val result = MorphineCompiler(
            text = CTypeConversion.toJavaString(text),
            optimize = optimize
        ).compile()

        assembleResult = when (result) {
            is MorphineCompiler.Result.Error -> Result.Error(result.message)
            is MorphineCompiler.Result.Success -> Result.Success(
                binary = result.data
            )
        }

        return result is MorphineCompiler.Result.Error
    }

    @JvmStatic
    @CEntryPoint(name = "libcompiler_geterror")
    private fun getError(thread: IsolateThread) = CTypeConversion.toCString(
        (assembleResult as? Result.Error)?.message
    ).get()

    @JvmStatic
    @CEntryPoint(name = "libcompiler_getbytecodesize")
    private fun getBytecodeSize(thread: IsolateThread) = (assembleResult as? Result.Success)?.binary?.size ?: 0

    @JvmStatic
    @CEntryPoint(name = "libcompiler_getbytecodevector")
    private fun getBytecodeVector(thread: IsolateThread) = CTypeConversion.toCBytes(
        (assembleResult as? Result.Success)?.binary?.toByteArray()
    ).get()

    @JvmStatic
    @CEntryPoint(name = "libcompiler_version")
    private fun version(thread: IsolateThread) =
        CTypeConversion.toCString(BuildConfig.version).get()

    @JvmStatic
    @CEntryPoint(name = "libcompiler_versioncode")
    private fun versionCode(thread: IsolateThread) = BuildConfig.versionCode

    private sealed interface Result {

        data class Success(
            val binary: List<Byte>
        ) : Result

        data class Error(
            val message: String
        ) : Result
    }
}