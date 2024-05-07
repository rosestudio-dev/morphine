package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.bytecode.Value
import ru.unit.morphine.assembly.compiler.ast.node.ReturnStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.ast.node.ValueExpression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementLeave(): Statement {
    val saved = position
    consume(Token.SystemWord.LEAVE)

    return ReturnStatement(
        expression = ValueExpression(Value.Nil, data(saved)),
        data = data(saved)
    )
}