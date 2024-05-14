package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionConcat(): Expression {
    var result = expressionAdditive()

    while (true) {
        if (match(Token.Operator.DOTDOT)) {
            val saved = position - 1
            result = BinaryExpression(
                type = BinaryExpression.Type.CONCAT,
                expressionA = result,
                expressionB = expressionAdditive(),
                data = data(saved)
            )
        } else {
            break
        }
    }

    return result
}