package morphine.compiler.parser.functions

import morphine.compiler.ast.node.IteratorStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementIterator(): Statement {
    val saved = position

    consume(Token.SystemWord.ITERATOR)

    consume(Token.Operator.LPAREN)
    val method = supportAssignMethod {
        consumeWord().text
    }

    consume(Token.SystemWord.IN)

    val expression = expression()

    consume(Token.Operator.RPAREN)

    val statement = statementBlock(saved = saved)

    return IteratorStatement(
        method = method,
        iterable = expression,
        statement = statement,
        data = data(saved)
    )
}