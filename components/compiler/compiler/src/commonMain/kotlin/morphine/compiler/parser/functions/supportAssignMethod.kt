package morphine.compiler.parser.functions

import morphine.bytecode.Value
import morphine.compiler.ast.node.AssignMethod
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun <T> Parser.Controller.supportAssignMethod(
    parse: () -> T
): AssignMethod<T> = if (match(Token.SystemWord.DECOMPOSE)) {
    val saved = position

    val values = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LPAREN,
        close = Token.Operator.RPAREN,
        consumeOpen = false,
        consumeClose = false,
        parse = parse
    ).ifEmpty {
        throw ParseException("Empty decompose", data(saved - 1))
    }

    val keys = if (match(Token.SystemWord.AS)) {
        supportArguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LPAREN,
            close = Token.Operator.RPAREN,
            consumeOpen = false,
            consumeClose = false,
        ) { expression() }
    } else {
        List(values.size) { index ->
            ValueExpression(
                value = Value.Integer(index),
                data = data(saved)
            )
        }
    }

    val entries = values.mapIndexed { index, value ->
        val key = keys.getOrNull(index) ?:
            throw ParseException("Key for ${index + 1} decompose entry wasn't defined", data(saved - 1))

        AssignMethod.Decompose.Entry(
            value = value,
            key = key
        )
    }

    AssignMethod.Decompose(entries)
} else {
    AssignMethod.Single(
        entry = parse()
    )
}