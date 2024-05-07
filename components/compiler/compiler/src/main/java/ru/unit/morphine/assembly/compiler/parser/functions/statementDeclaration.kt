package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.DeclarationStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

fun Parser.Controller.statementDeclaration(): Statement {
    val saved = position

    val isMutable = when {
        match(Token.SystemWord.VAL) -> false
        match(Token.SystemWord.VAR) -> true
        else -> throw ParseException("Expected ${Token.SystemWord.VAL} or ${Token.SystemWord.VAR}", data(saved))
    }

    val method = supportAssignMethod { consumeWord().text }

    consume(Token.Operator.EQ)

    val expression = expression()

    return DeclarationStatement(
        method = method,
        isMutable = isMutable,
        expression = expression,
        data = data(saved)
    )
}