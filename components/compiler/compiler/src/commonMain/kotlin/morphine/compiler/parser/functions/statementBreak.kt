package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BreakStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementBreak(): Statement {
    val saved = position
    consume(Token.SystemWord.BREAK)
    return BreakStatement(data(saved))
}