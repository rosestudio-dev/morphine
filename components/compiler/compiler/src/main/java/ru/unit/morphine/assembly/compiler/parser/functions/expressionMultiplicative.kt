package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionMultiplicative(): Expression {
    var result = expressionPrefix()

    while (true) {
        when {
            match(Token.Operator.STAR) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.MUL,
                    expressionA = result,
                    expressionB = expressionPrefix(),
                    data = data(saved)
                )
            }

            match(Token.Operator.SLASH) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.DIV,
                    expressionA = result,
                    expressionB = expressionPrefix(),
                    data = data(saved)
                )
            }

            match(Token.Operator.PERCENT) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.MOD,
                    expressionA = result,
                    expressionB = expressionPrefix(),
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}