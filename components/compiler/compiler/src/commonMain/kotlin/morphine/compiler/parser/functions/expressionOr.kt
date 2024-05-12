package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionOr(): Expression {
    val result = expressionAnd()

    return if (match(Token.SystemWord.OR)) {
        val saved = position - 1
        BinaryExpression(
            type = BinaryExpression.Type.OR,
            expressionA = result,
            expressionB = expressionOr(),
            data = data(saved)
        )
    } else {
        result
    }
}