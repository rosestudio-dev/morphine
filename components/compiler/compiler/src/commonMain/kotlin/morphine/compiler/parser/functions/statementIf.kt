package morphine.compiler.parser.functions

import morphine.compiler.ast.node.EmptyStatement
import morphine.compiler.ast.node.IfStatement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementIf(elif: Boolean = false): IfStatement {
    val saved = position

    if (elif) {
        consume(Token.SystemWord.ELIF)
    } else {
        consume(Token.SystemWord.IF)
    }

    consume(Token.Operator.LPAREN)
    val condition = expression()
    consume(Token.Operator.RPAREN)

    val ifStatement = statementBlock(
        saved = saved,
        Token.SystemWord.ELSE,
        Token.SystemWord.ELIF,
        Token.SystemWord.END
    )

    val elseStatement = when {
        look(Token.SystemWord.ELIF) -> statementIf(elif = true)
        match(Token.SystemWord.ELSE) -> statementBlock(saved = position - 1)
        else -> {
            match(Token.SystemWord.END)
            EmptyStatement(data(saved))
        }
    }

    return IfStatement(
        condition = condition,
        ifStatement = ifStatement,
        elseStatement = elseStatement,
        data(saved)
    )
}