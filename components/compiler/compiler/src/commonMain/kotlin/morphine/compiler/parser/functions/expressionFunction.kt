package morphine.compiler.parser.functions

import morphine.compiler.ast.node.FunctionExpression
import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

fun Parser.Controller.expressionFunction(
    requireName: Boolean = false
): FunctionExpression {
    val saved = position

    consume(Token.SystemWord.FUN)

    val isRecursive = match(Token.SystemWord.RECURSIVE)

    val name = if (lookWord() || requireName) {
        consumeWord().text
    } else if (isRecursive) {
        throw ParseException("Recursive function needs name", data(saved))
    } else {
        null
    }

    val closures = if (look(Token.Operator.LT)) {
        supportArguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LT,
            close = Token.Operator.GT
        ) {
            val accessName = consumeWord().text

            if (match(Token.SystemWord.AS)) {
                val aliasName = consumeWord().text

                FunctionExpression.Closure(
                    access = accessName,
                    alias = aliasName
                )
            } else {
                FunctionExpression.Closure(
                    access = accessName,
                    alias = accessName
                )
            }
        }
    } else {
        emptyList()
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
        isRecursive = isRecursive,
        closures = closures,
        statement = statement,
        data = data(saved)
    )
}