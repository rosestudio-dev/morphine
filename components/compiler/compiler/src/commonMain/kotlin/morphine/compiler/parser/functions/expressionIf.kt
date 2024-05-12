package morphine.compiler.parser.functions

import morphine.compiler.ast.node.IfExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionIf(elif: Boolean = false): IfExpression {
    val saved = position

    if (elif) {
        consume(Token.SystemWord.ELIF)
    } else {
        consume(Token.SystemWord.IF)
    }

    consume(Token.Operator.LPAREN)
    val condition = expression()
    consume(Token.Operator.RPAREN)

    val ifStatement = expressionBlock(
        saved = saved,
        Token.SystemWord.ELSE,
        Token.SystemWord.ELIF,
        Token.SystemWord.END
    )

    val elseStatement = when {
        look(Token.SystemWord.ELIF) -> expressionIf(elif = true)
        else -> {
            consume(Token.SystemWord.ELSE)
            expressionBlock(saved = position - 1)
        }
    }

    return IfExpression(
        condition = condition,
        ifExpression = ifStatement,
        elseExpression = elseStatement,
        data(saved)
    )
}
