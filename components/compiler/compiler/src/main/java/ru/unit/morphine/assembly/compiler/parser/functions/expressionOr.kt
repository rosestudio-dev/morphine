package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionOr(): Expression {
    val result = expressionAnd()

    return if (match(Token.SystemWord.OR)) {
        val saved = position - 1
        BinaryExpression(
            type = BinaryExpression.Type.OR,
            expressionA = result,
            expressionB = expressionOr(),
            data = data(saved)
        )
    } else {
        result
    }
}