package ru.unit.morphine.assembly.compiler

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.compiler.ast.compiler.exception.CompilerException
import ru.unit.morphine.assembly.compiler.lexer.Lexer
import ru.unit.morphine.assembly.compiler.lexer.exception.LexerException
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException
import ru.unit.morphine.assembly.optimizer.Optimizer

class MorphineAssemble(
    private val text: String,
    private val optimize: Boolean,
    private val print: Boolean = false,
    private val debug: Boolean = false,
) {

    fun assemble() = runCatching {
        Result.Success(unsafeAssemble())
    }.getOrElse { throwable ->
        if (debug) {
            throw throwable
        }

        when (throwable) {
            is LexerException -> Result.Error("Lexer: " + (throwable.message ?: "No message"))
            is ParseException -> {
                val message = throwable.message ?: "No message"
                val newline = if (message.contains('\n')) {
                    "\n"
                } else {
                    " "
                }

                Result.Error("Parser:$newline$message")
            }

            is CompilerException -> Result.Error("Compiler: ${throwable.messageWithLineData}")
            else -> Result.Error("Internal undefined exception")
        }
    }

    private fun unsafeAssemble(): Bytecode {
        val tokens = Lexer(text).tokenize()
        val ast = Parser(tokens = tokens, debug = debug).parse()
        val bytecode = ast.compile(optimize)
        val optimized = Optimizer(debug).optimize(bytecode)

        if (debug) {
            Printer.tokens(tokens)
            Printer.bytecode(bytecode, text)
        }

        if (print || debug) {
            Printer.bytecode(optimized, text)
        }

        return optimized
    }

    sealed interface Result {

        data class Success(val bytecode: Bytecode) : Result
        data class Error(val message: String) : Result
    }
}