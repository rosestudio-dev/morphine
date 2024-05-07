package ru.unit.morphine.assembly.compiler.parser

import kotlin.math.max
import kotlin.math.min
import ru.unit.morphine.assembly.bytecode.LineData
import ru.unit.morphine.assembly.compiler.ast.Ast
import ru.unit.morphine.assembly.compiler.ast.node.BlockStatement
import ru.unit.morphine.assembly.compiler.ast.node.Node
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.LinedToken
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException
import ru.unit.morphine.assembly.compiler.parser.functions.statement

class Parser(private val tokens: List<LinedToken>) {

    fun parse() = Controller(tokens).ast()

    private fun Controller.ast(): Ast {
        val list = mutableListOf<Statement>()

        while (!match(Token.Eof)) {
            list.add(statement())
        }

        val statement = BlockStatement(
            statements = list,
            data = data(0)
        )

        return Ast(statement)
    }

    inner class Controller(
        private val tokens: List<LinedToken>
    ) {

        var position = 0
            private set

        fun rollback(saved: Int) {
            position = max(0, min(saved, tokens.size - 1))
        }

        fun get(relative: Int = 0) =
            tokens.getOrNull(position + relative)?.token ?: Token.Eof

        fun match(vararg tokens: Token) = look(*tokens).also { result ->
            if (result) {
                position++
            }
        }

        fun consume(token: Token): Token {
            val current = get()

            return if (current == token) {
                position++
                current
            } else {
                throw ParseException("Token $current doesn't match $token", data(position))
            }
        }

        fun look(vararg tokens: Token, relative: Int = 0) =
            tokens.any { token -> get(relative) == token }

        fun consumeWord(): Token.Word {
            val current = get()

            return if (current is Token.Word) {
                position++
                current
            } else {
                throw ParseException("Token $current doesn't match Word", data(position))
            }
        }

        fun lookWord() = get() is Token.Word

        fun consumeText(): Token.Text {
            val current = get()

            return if (current is Token.Text) {
                position++
                current
            } else {
                throw ParseException("Token $current doesn't match Text", data(position))
            }
        }

        fun lookText() = get() is Token.Text

        fun consumeNumber(): Token.Number {
            val current = get()

            return if (current is Token.Number) {
                position++
                current
            } else {
                throw ParseException("Token $current doesn't match Number", data(position))
            }
        }

        fun lookNumber() = get() is Token.Number

        fun data(saved: Int) = Node.Data(
            lineData = tokens.getOrNull(saved)?.lineData
                ?: tokens.lastOrNull()?.lineData
                ?: LineData(line = 0, column = 0)
        )
    }
}