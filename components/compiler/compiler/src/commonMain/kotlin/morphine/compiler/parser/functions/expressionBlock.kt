package morphine.compiler.parser.functions

import morphine.compiler.ast.node.BlockExpression
import morphine.compiler.ast.node.EvalStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun Parser.Controller.expressionBlock(
    saved: Int,
    vararg closes: Token,
): BlockExpression {
    val list = mutableListOf<Statement>()
    while (!look(*closes) && !match(Token.SystemWord.END)) {
        val rollbackPosition = position
        val expression = runCatching {
            statement(allowAllExpressions = true)
        }

        if (look(*closes) || look(Token.SystemWord.END)) {
            list.add(expression.getOrThrow())
        } else {
            rollback(rollbackPosition)
            list.add(statement())
        }
    }

    if (list.isEmpty()) {
        throw ParseException("Expression block is empty", data(saved))
    }

    val expression = when (val last = list.removeLast()) {
        is EvalStatement -> last.expression
        else -> throw ParseException("Expression block must be ended with expression", data(saved))
    }

    return BlockExpression(
        statements = list,
        expression = expression,
        data = data(saved)
    )
}