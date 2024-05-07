package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.Accessible
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.ast.node.IncrementExpression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

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