package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.YieldStatement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementYield(): YieldStatement {
    val saved = position

    consume(Token.SystemWord.YIELD)

    return YieldStatement(data(saved))
}