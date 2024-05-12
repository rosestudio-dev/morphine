package morphine.compiler.parser.functions

import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

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