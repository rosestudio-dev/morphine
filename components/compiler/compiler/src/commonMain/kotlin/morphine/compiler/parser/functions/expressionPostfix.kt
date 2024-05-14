package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Accessible
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.IncrementExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun Parser.Controller.expressionPostfix(): Expression {
    var expression = expressionPrimary()

    while (true) {
        when {
            match(Token.Operator.PLUSPLUS) -> expression = IncrementExpression(
                type = IncrementExpression.Type.INCREMENT,
                isPostfix = true,
                accessible = expression as? Accessible
                    ?: throw ParseException("Increment requires accessible expression", data(position - 1)),
                data = data(position - 1)
            )

            match(Token.Operator.MINUSMINUS) -> expression = IncrementExpression(
                type = IncrementExpression.Type.DECREMENT,
                isPostfix = true,
                accessible = expression as? Accessible
                    ?: throw ParseException("Decrement requires accessible expression", data(position - 1)),
                data = data(position - 1)
            )

            else -> break
        }
    }

    return expression
}