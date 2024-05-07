package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.ast.node.VectorExpression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionVector(): Expression {
    val saved = position

    val elements = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LBRACKET,
        close = Token.Operator.RBRACKET
    ) { expression() }

    return VectorExpression(
        elements = elements,
        data = data(saved)
    )
}