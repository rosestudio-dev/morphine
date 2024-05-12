package morphine.compiler.parser.functions

import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementReturn(): Statement {
    val saved = position
    consume(Token.SystemWord.RETURN)

    return ReturnStatement(
        expression = expression(),
        data = data(saved)
    )
}