package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionAnd(): Expression {
    val result = expressionEqual()

    return if (match(Token.SystemWord.AND)) {
        val saved = position - 1
        BinaryExpression(
            type = BinaryExpression.Type.AND,
            expressionA = result,
            expressionB = expressionAnd(),
            data = data(saved)
        )
    } else {
        result
    }
}