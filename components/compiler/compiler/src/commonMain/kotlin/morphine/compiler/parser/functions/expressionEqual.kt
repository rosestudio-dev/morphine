package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.UnaryExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionEqual(): Expression {
    var result = expressionConditional()

    while (true) {
        when {
            match(Token.Operator.EQEQ) -> {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.EQUALS,
                    expressionA = result,
                    expressionB = expressionConditional(),
                    data = data(saved)
                )
            }

            match(Token.Operator.EXCLEQ) -> {
                val saved = position - 1
                result = UnaryExpression(
                    type = UnaryExpression.Type.NOT,
                    expression = BinaryExpression(
                        type = BinaryExpression.Type.EQUALS,
                        expressionA = result,
                        expressionB = expressionConditional(),
                        data = data(saved)
                    ),
                    data = data(saved)
                )
            }

            else -> break
        }
    }

    return result
}