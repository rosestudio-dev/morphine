package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionAdditive(): Expression {
    var result = expressionMultiplicative()

    while (true) {
        when {
            match(Token.Operator.PLUS) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.ADD,
                    expressionA = result,
                    expressionB = expressionMultiplicative(),
                    data = data(saved)
                )
            }

            match(Token.Operator.MINUS) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.SUB,
                    expressionA = result,
                    expressionB = expressionMultiplicative(),
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}