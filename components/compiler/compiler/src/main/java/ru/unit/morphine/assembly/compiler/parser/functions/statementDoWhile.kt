package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.DoWhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementDoWhile(
    allowBlock: Boolean
): Statement {
    val saved = position

    consume(Token.SystemWord.DO)

    val statement = statementBlock(saved = saved)

    return if (match(Token.SystemWord.WHILE)) {
        consume(Token.Operator.LPAREN)
        val condition = expression()
        consume(Token.Operator.RPAREN)

        DoWhileStatement(
            condition = condition,
            statement = statement,
            data = data(saved)
        )
    } else {
        if (!allowBlock) {
            consume(Token.SystemWord.WHILE)
        }

        statement
    }
}