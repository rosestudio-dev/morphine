package morphine.compiler.parser.functions

import morphine.compiler.ast.node.FunctionExpression
import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionFunction(
    requireName: Boolean = false
): FunctionExpression {
    val saved = position

    consume(Token.SystemWord.FUN)

    val name = if (lookWord() || requireName) {
        consumeWord().text
    } else {
        null
    }

    val arguments = supportArguments(
        determinator = Token.Operator.COMMA,
        open = Token.Operator.LPAREN,
        close = Token.Operator.RPAREN
    ) {
        consumeWord().text
    }

    val statics = if (match(Token.SystemWord.STATIC)) {
        supportArguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LPAREN,
            close = Token.Operator.RPAREN
        ) {
            consumeWord().text
        }
    } else {
        emptyList()
    }

    val statement = if (match(Token.Operator.EQ)) {
        ReturnStatement(
            expression = expression(),
            data = data(position - 1),
        )
    } else {
        statementBlock(saved = saved)
    }

    return FunctionExpression(
        name = name,
        statics = statics,
        arguments = arguments,
        statement = statement,
        data = data(saved)
    )
}