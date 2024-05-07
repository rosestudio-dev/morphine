package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BlockExpression
import ru.unit.morphine.assembly.compiler.ast.node.EvalStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

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