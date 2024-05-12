package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionAdditive(): Expression {
    var result = expressionMultiplicative()

    while (true) {
        when {
            match(Token.Operator.PLUS) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.ADD,
                    expressionA = result,
                    expressionB = expressionMultiplicative(),
                    data = data(saved)
                )
            }

            match(Token.Operator.MINUS) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.SUB,
                    expressionA = result,
                    expressionB = expressionMultiplicative(),
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}