package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.FunctionExpression
import ru.unit.morphine.assembly.compiler.ast.node.ReturnStatement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionFunction(
    requireName: Boolean = false
): FunctionExpression {
    val saved = position

    consume(Token.SystemWord.FUN)

    val name = if (lookWord() || requireName) {
        consumeWord().text
    } else {
        null
    }

    val arguments = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LPAREN,
        close = Token.Operator.RPAREN
    ) {
        consumeWord().text
    }

    val statics = if (match(Token.SystemWord.STATIC)) {
        supportArguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LPAREN,
            close = Token.Operator.RPAREN
        ) {
            consumeWord().text
        }
    } else {
        emptyList()
    }

    val statement = if (match(Token.Operator.EQ)) {
        ReturnStatement(
            expression = expression(),
            data = data(position - 1),
        )
    } else {
        statementBlock(saved = saved)
    }

    return FunctionExpression(
        name = name,
        statics = statics,
        arguments = arguments,
        statement = statement,
        data = data(saved)
    )
}