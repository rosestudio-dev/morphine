package morphine.compiler.parser.functions

import morphine.compiler.ast.node.DoWhileStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

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