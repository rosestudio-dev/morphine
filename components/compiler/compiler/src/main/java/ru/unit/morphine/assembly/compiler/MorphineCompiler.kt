package ru.unit.morphine.assembly.compiler

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.bytecode.BytecodeConverter
import ru.unit.morphine.assembly.compiler.ast.compilerold.exception.CompilerException
import ru.unit.morphine.assembly.compiler.lexer.Lexer
import ru.unit.morphine.assembly.compiler.lexer.exception.LexerException
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException
import ru.unit.morphine.assembly.optimizer.Optimizer

class MorphineCompiler(
    private val text: String,
    private val optimize: Boolean,
) {

    fun assemble() = runCatching {
        val bytecode = unsafeAssemble()

        Result.Success(
            data = bytecode,
            bytecode = bytecode
        )
    }.getOrElse { throwable ->
        parseThrowable(throwable)
    }

    fun compile() = runCatching {
        val bytecode = unsafeAssemble()
        val bytes = BytecodeConverter().convert(bytecode)

        Result.Success(
            data = bytes,
            bytecode = bytecode
        )
    }.getOrElse { throwable ->
        parseThrowable(throwable)
    }

    private fun <T> parseThrowable(throwable: Throwable): Result.Error<T> {
        return when (throwable) {
            is LexerException -> Result.Error(
                message = "Lexer: ${throwable.message}",
                throwable = throwable
            )

            is ParseException -> Result.Error(
                message = "Parser: ${throwable.message}",
                throwable = throwable
            )

            is CompilerException -> Result.Error(
                message = "Compiler: ${throwable.messageWithLineData}",
                throwable = throwable
            )

            is BytecodeConverter.ConvertException -> Result.Error(
                message = "Binary: ${throwable.message}",
                throwable = throwable
            )

            else -> Result.Error(
                message = "Internal undefined exception",
                throwable = throwable
            )
        }
    }

    private fun unsafeAssemble(): Bytecode {
        val tokens = Lexer(text).tokenize()
        val ast = Parser(tokens).parse()
        val bytecode = ast.compile(optimize)
        val optimized = Optimizer(bytecode).optimize()

        return optimized
    }

    sealed interface Result<T> {

        data class Success<T>(val data: T, val bytecode: Bytecode) : Result<T>
        data class Error<T>(val message: String, val throwable: Throwable) : Result<T>
    }
}