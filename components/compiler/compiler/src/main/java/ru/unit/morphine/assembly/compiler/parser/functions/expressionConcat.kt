package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionConcat(): Expression {
    var result = expressionAdditive()

    while (true) {
        if (match(Token.Operator.DOTDOT)) {
            val saved = position - 1
            result = BinaryExpression(
                type = BinaryExpression.Type.CONCAT,
                expressionA = result,
                expressionB = expressionAdditive(),
                data = data(saved)
            )
        } else {
            break
        }
    }

    return result
}