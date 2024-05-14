package morphine.compiler.parser.functions

import morphine.compiler.ast.node.YieldStatement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementYield(): YieldStatement {
    val saved = position

    consume(Token.SystemWord.YIELD)

    return YieldStatement(data(saved))
}