package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionConditional(): Expression {
    var result = expressionConcat()

    while (true) {
        when {
            match(Token.Operator.LT) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.LESS,
                    expressionA = result,
                    expressionB = expressionConcat(),
                    data = data(saved)
                )
            }

            match(Token.Operator.GT) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.LESS,
                    expressionA = expressionConcat(),
                    expressionB = result,
                    data = data(saved)
                )
            }

            match(Token.Operator.LTEQ) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.LESS_EQUALS,
                    expressionA = result,
                    expressionB = expressionConcat(),
                    data = data(saved)
                )
            }

            match(Token.Operator.GTEQ) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.LESS_EQUALS,
                    expressionA = expressionConcat(),
                    expressionB = result,
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}