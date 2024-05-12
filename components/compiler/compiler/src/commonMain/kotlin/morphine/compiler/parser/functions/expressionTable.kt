package morphine.compiler.parser.functions

import morphine.bytecode.Value
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.TableExpression
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionTable(): Expression {
    val saved = position

    var index = 0

    val elements = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LBRACE,
        close = Token.Operator.RBRACE
    ) {
        val expression = expression()

        val element = if (match(Token.SystemWord.TO)) {
            TableExpression.Element(
                key = expression,
                value = expression()
            )
        } else {
            TableExpression.Element(
                key = ValueExpression(
                    value = Value.Integer(index),
                    data = data(saved)
                ),
                value = expression
            )
        }

        index++

        element
    }

    return TableExpression(
        elements = elements,
        data = data(saved)
    )
}