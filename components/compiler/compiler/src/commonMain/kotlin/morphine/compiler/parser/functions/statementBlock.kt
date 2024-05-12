package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BlockStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementBlock(
    saved: Int,
    vararg closes: Token
): BlockStatement {
    val list = mutableListOf<Statement>()

    while (!look(*closes) && !match(Token.SystemWord.END)) {
        list.add(statement())
    }

    return BlockStatement(list, data(saved))
}