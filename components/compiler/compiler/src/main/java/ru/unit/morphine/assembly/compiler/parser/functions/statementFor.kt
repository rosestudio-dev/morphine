package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.ForStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementFor(): Statement {
    val saved = position

    consume(Token.SystemWord.FOR)

    consume(Token.Operator.LPAREN)

    val initial = statement(allowSemicolon = false)
    consume(Token.Operator.SEMICOLON)
    val condition = expression()
    consume(Token.Operator.SEMICOLON)
    val iterator = statement(allowSemicolon = false)

    consume(Token.Operator.RPAREN)

    val statement = statementBlock(saved = saved)

    return ForStatement(
        initial = initial,
        condition = condition,
        iterator = iterator,
        statement = statement,
        data = data(saved)
    )
}