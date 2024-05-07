package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun <T> Parser.Controller.supportArguments(
    determinator: Token,
    open: Token? = null,
    close: Token? = null,
    consumeOpen: Boolean = true,
    consumeClose: Boolean = true,
    parse: () -> T
): List<T> {
    open?.let {
        if (consumeOpen) {
            consume(open)
        } else {
            match(open)
        }
    }

    if (close != null && match(close)) {
        return emptyList()
    }

    val result = mutableListOf(parse())

    while (match(determinator)) {
        if (close != null && match(close)) {
            return result
        }

        result.add(parse())
    }

    close?.let {
        if (consumeClose) {
            consume(close)
        } else {
            match(close)
        }
    }

    return result
}