package morphine.compiler.parser.functions

import morphine.bytecode.Value
import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementLeave(): Statement {
    val saved = position
    consume(Token.SystemWord.LEAVE)

    return ReturnStatement(
        expression = ValueExpression(Value.Nil, data(saved)),
        data = data(saved)
    )
}