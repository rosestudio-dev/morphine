package morphine.compiler.parser.functions

import morphine.compiler.ast.node.ContinueStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementContinue(): Statement {
    val saved = position
    consume(Token.SystemWord.CONTINUE)
    return ContinueStatement(data(saved))
}