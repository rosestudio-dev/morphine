package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionAnd(): Expression {
    val result = expressionEqual()

    return if (match(Token.SystemWord.AND)) {
        val saved = position - 1
        BinaryExpression(
            type = BinaryExpression.Type.AND,
            expressionA = result,
            expressionB = expressionAnd(),
            data = data(saved)
        )
    } else {
        result
    }
}