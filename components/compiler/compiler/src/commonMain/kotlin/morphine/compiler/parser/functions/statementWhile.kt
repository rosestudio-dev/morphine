package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Statement
import morphine.compiler.ast.node.WhileStatement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

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