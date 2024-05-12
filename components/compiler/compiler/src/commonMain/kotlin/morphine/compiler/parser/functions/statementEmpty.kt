package morphine.compiler.parser.functions

import morphine.compiler.ast.node.EmptyStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementEmpty(): Statement {
    val saved = position
    consume(Token.SystemWord.PASS)
    return EmptyStatement(data(saved))
}