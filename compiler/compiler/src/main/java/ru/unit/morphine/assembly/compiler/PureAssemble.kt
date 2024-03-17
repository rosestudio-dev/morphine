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

    fun assemble(): Bytecode {
        val tokens = runCatching {
            Lexer(text).tokenize()
        }.getOrElse { throwable ->
            throw if (throwable is LexerException && !debug) {
                println("Lexer: ${throwable.message}")
                Exception()
            } else {
                throwable
            }
        }

        val ast = runCatching {
            Parser(tokens = tokens, debug = debug).parse()
        }.getOrElse { throwable ->
            throw if (throwable is ParseException && !debug) {
                println(throwable.message)
                Exception()
            } else {
                throwable
            }
        }

        val bytecode = runCatching {
            ast.compile(optimize)
        }.getOrElse { throwable ->
            throw if (throwable is CompilerException && !debug) {
                println("${throwable.lineData?.toString()?.plus(" ") ?: ""}${throwable.message}")
                Exception()
            } else {
                throwable
            }
        }

        val optimized = Optimizer(debug).optimize(bytecode)

        if (debug) {
            Printer.tokens(tokens)
            Printer.ast(ast)
            Printer.bytecode(bytecode, text)
        }

        if (print || debug) {
            Printer.bytecode(optimized, text)
        }

        return optimized
    }
}