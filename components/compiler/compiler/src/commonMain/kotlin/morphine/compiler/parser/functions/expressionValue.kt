package morphine.compiler.parser.functions

import morphine.bytecode.Value
import morphine.compiler.ast.node.EnvExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.SelfExpression
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.ast.node.VariableAccessible
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun Parser.Controller.expressionValue(): Expression = when {
    look(Token.SystemWord.NIL) -> ValueExpression(
        value = Value.Nil,
        data = data(position)
    ).also { consume(Token.SystemWord.NIL) }

    lookNumber() -> ValueExpression(
        value = (get() as Token.Number).number.let { text ->
            if (text.contains(".")) {
                runCatching {
                    Value.Decimal(text.toDouble())
                }.getOrElse {
                    throw ParseException("$text isn't decimal", data(position))
                }
            } else {
                runCatching {
                    Value.Integer(text.toInt())
                }.getOrElse {
                    throw ParseException("$text isn't integer", data(position))
                }
            }
        },
        data = data(position)
    ).also { consumeNumber() }

    lookText() -> ValueExpression(
        value = Value.String((get() as Token.Text).text),
        data = data(position)
    ).also { consumeText() }

    match(Token.SystemWord.TRUE) -> ValueExpression(
        value = Value.Boolean(true),
        data = data(position)
    )

    match(Token.SystemWord.FALSE) -> ValueExpression(
        value = Value.Boolean(false),
        data = data(position)
    )

    lookWord() -> VariableAccessible(
        name = (get() as Token.Word).text,
        data = data(position)
    ).also { consumeWord() }

    look(Token.SystemWord.ENV) -> EnvExpression(
        data = data(position)
    ).also { consume(Token.SystemWord.ENV) }

    look(Token.SystemWord.SELF) -> SelfExpression(
        data = data(position)
    ).also { consume(Token.SystemWord.SELF) }

    look(Token.Operator.LBRACE) -> expressionTable()
    look(Token.Operator.LBRACKET) -> expressionVector()

    match(Token.Operator.LPAREN) -> expression().also { consume(Token.Operator.RPAREN) }

    else -> throw ParseException("Expected expression", data(position))
}