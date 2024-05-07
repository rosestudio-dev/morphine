package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.Accessible
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.ast.node.IncrementExpression
import ru.unit.morphine.assembly.compiler.ast.node.UnaryExpression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

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