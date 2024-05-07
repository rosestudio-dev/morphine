package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.ReturnStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementReturn(): Statement {
    val saved = position
    consume(Token.SystemWord.RETURN)

    return ReturnStatement(
        expression = expression(),
        data = data(saved)
    )
}