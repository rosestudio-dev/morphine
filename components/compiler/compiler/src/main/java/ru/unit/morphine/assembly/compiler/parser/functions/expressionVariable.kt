package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.bytecode.Value
import ru.unit.morphine.assembly.compiler.ast.node.AccessAccessible
import ru.unit.morphine.assembly.compiler.ast.node.CallExpression
import ru.unit.morphine.assembly.compiler.ast.node.CallSelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.ast.node.ValueExpression
import ru.unit.morphine.assembly.compiler.ast.node.VariableAccessible
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionVariable(): Expression {
    var result = expressionValue()

    while (true) {
        when {
            match(Token.Operator.LBRACKET) -> {
                val saved = position - 1
                result = AccessAccessible(
                    container = result,
                    key = expression(),
                    data = data(saved)
                )

                consume(Token.Operator.RBRACKET)
            }

            match(Token.Operator.DOT) -> {
                val saved = position - 1

                val text = consumeWord().text
                result = AccessAccessible(
                    container = result,
                    key = ValueExpression(Value.String(text), data(saved + 1)),
                    data = data(saved)
                )
            }

            look(Token.Operator.LPAREN) || look(Token.Operator.LBRACE) -> {
                val saved = position - 1
                val arguments = functionArguments()

                result = CallExpression(
                    expression = result,
                    arguments = arguments,
                    data = data(saved)
                )
            }

            match(Token.Operator.COLON) -> {
                val saved = position - 1

                val callable = when {
                    match(Token.Operator.LBRACKET) -> {
                        expression().also {
                            consume(Token.Operator.RBRACKET)
                        }
                    }

                    else -> {
                        val text = consumeWord().text

                        ValueExpression(
                            value = Value.String(text),
                            data = data(saved + 1)
                        )
                    }
                }

                val arguments = functionArguments()

                result = CallSelfExpression(
                    self = result,
                    callable = callable,
                    extractCallable = true,
                    arguments = arguments,
                    data = data(saved)
                )
            }

            match(Token.Operator.RARROW) -> {
                val saved = position - 1

                val callable = when {
                    match(Token.Operator.LBRACKET) -> {
                        expression().also {
                            consume(Token.Operator.RBRACKET)
                        }
                    }

                    else -> {
                        val text = consumeWord().text

                        VariableAccessible(
                            name = text,
                            data = data(saved + 1)
                        )
                    }
                }

                val arguments = functionArguments()

                result = CallSelfExpression(
                    self = result,
                    callable = callable,
                    extractCallable = false,
                    arguments = arguments,
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}

private fun Parser.Controller.functionArguments() = if (look(Token.Operator.LBRACE)) {
    listOf(expressionTable())
} else {
    val arguments = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LPAREN,
        close = Token.Operator.RPAREN
    ) { expression() }

    val table = if (look(Token.Operator.LBRACE)) {
        listOf(expressionTable())
    } else {
        emptyList()
    }

    arguments + table
}
