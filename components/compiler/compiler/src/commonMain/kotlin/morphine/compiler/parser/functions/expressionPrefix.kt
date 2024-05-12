package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Accessible
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.IncrementExpression
import morphine.compiler.ast.node.UnaryExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun Parser.Controller.expressionPrefix(): Expression {
    val saved = position

    return when {
        match(Token.SystemWord.NOT) -> UnaryExpression(
            type = UnaryExpression.Type.NOT,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.Operator.MINUS) -> UnaryExpression(
            type = UnaryExpression.Type.NEGATE,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.SystemWord.TYPE) -> UnaryExpression(
            type = UnaryExpression.Type.TYPE,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.SystemWord.LEN) -> UnaryExpression(
            type = UnaryExpression.Type.LEN,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.SystemWord.REF) -> UnaryExpression(
            type = UnaryExpression.Type.REF,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.Operator.STAR) -> UnaryExpression(
            type = UnaryExpression.Type.DEREF,
            expression = expressionPrefix(),
            data = data(saved)
        )

        match(Token.Operator.PLUSPLUS) -> IncrementExpression(
            type = IncrementExpression.Type.INCREMENT,
            isPostfix = false,
            accessible = expressionPrefix() as? Accessible
                ?: throw ParseException("Increment requires accessible expression", data(saved)),
            data = data(saved)
        )

        match(Token.Operator.MINUSMINUS) -> IncrementExpression(
            type = IncrementExpression.Type.DECREMENT,
            isPostfix = false,
            accessible = expressionPrefix() as? Accessible
                ?: throw ParseException("Decrement requires accessible expression", data(saved)),
            data = data(saved)
        )


        else -> expressionPostfix()
    }
}