package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.VectorExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionVector(): Expression {
    val saved = position

    val elements = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LBRACKET,
        close = Token.Operator.RBRACKET
    ) { expression() }

    return VectorExpression(
        elements = elements,
        data = data(saved)
    )
}