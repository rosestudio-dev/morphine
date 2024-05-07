package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.IteratorStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementIterator(): Statement {
    val saved = position

    consume(Token.SystemWord.ITERATOR)

    consume(Token.Operator.LPAREN)
    val method = supportAssignMethod {
        consumeWord().text
    }

    consume(Token.SystemWord.IN)

    val expression = expression()

    consume(Token.Operator.RPAREN)

    val statement = statementBlock(saved = saved)

    return IteratorStatement(
        method = method,
        iterable = expression,
        statement = statement,
        data = data(saved)
    )
}