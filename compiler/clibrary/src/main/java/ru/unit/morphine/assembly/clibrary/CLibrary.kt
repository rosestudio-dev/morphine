package ru.unit.morphine.assembly.clibrary

import org.graalvm.nativeimage.IsolateThread
import org.graalvm.nativeimage.c.function.CEntryPoint
import org.graalvm.nativeimage.c.type.CCharPointer
import org.graalvm.nativeimage.c.type.CTypeConversion
import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.bytecode.BytecodeConverter
import ru.unit.morphine.assembly.compiler.MorphineAssemble

object CLibrary {

    private var assembleResult: Result? = null

    @JvmStatic
    @CEntryPoint(name = "morphinecompiler_assemble")
    private fun compile(
        thread: IsolateThread,
        text: CCharPointer,
        optimize: Boolean
    ): Boolean {
        val result = MorphineAssemble(
            text = CTypeConversion.toJavaString(text),
            optimize = optimize,
            print = false,
            debug = false
        ).assemble()

        assembleResult = when (result) {
            is MorphineAssemble.Result.Error -> Result.Error(result.message)
            is MorphineAssemble.Result.Success -> Result.Success(
                binary = BytecodeConverter().convert(result.bytecode),
                bytecode = result.bytecode
            )
        }

        return result is MorphineAssemble.Result.Error
    }

    @JvmStatic
    @CEntryPoint(name = "morphinecompiler_geterror")
    private fun getError(thread: IsolateThread) = CTypeConversion.toCString(
        (assembleResult as? Result.Error)?.message
    ).get()

    @JvmStatic
    @CEntryPoint(name = "morphinecompiler_getbytecodesize")
    private fun getBytecodeSize(thread: IsolateThread) = (assembleResult as? Result.Success)?.binary?.size ?: 0

    @JvmStatic
    @CEntryPoint(name = "morphinecompiler_getbytecodevector")
    private fun getBytecodeVector(thread: IsolateThread) = CTypeConversion.toCBytes(
        (assembleResult as? Result.Success)?.binary?.toByteArray()
    ).get()

    @JvmStatic
    @CEntryPoint(name = "morphinecompiler_version")
    private fun version(thread: IsolateThread) = CTypeConversion.toCString(
        this::class.java.`package`.implementationVersion
    ).get()

    private sealed interface Result {

        data class Success(
            val binary: List<Byte>,
            val bytecode: Bytecode
        ) : Result

        data class Error(
            val message: String
        ) : Result
    }
}