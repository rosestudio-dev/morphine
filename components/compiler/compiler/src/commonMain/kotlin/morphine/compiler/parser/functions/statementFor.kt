package morphine.compiler.parser.functions

import morphine.compiler.ast.node.ForStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

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