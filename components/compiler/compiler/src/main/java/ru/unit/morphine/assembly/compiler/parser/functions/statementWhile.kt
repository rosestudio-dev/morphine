package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.ast.node.WhileStatement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementWhile(): Statement {
    val saved = position

    consume(Token.SystemWord.WHILE)

    consume(Token.Operator.LPAREN)
    val condition = expression()
    consume(Token.Operator.RPAREN)

    val statement = statementBlock(saved = saved)

    return WhileStatement(
        condition = condition,
        statement = statement,
        data = data(saved)
    )
}