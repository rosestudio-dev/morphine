package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.BlockStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementBlock(
    saved: Int,
    vararg closes: Token
): BlockStatement {
    val list = mutableListOf<Statement>()

    while (!look(*closes) && !match(Token.SystemWord.END)) {
        list.add(statement())
    }

    return BlockStatement(list, data(saved))
}