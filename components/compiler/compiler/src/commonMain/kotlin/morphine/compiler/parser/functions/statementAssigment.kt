package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Accessible
import morphine.compiler.ast.node.AssigmentStatement
import morphine.compiler.ast.node.AssignMethod
import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.Expression
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser
import morphine.compiler.parser.exception.ParseException

private val ASSIGN_OPERATORS = mapOf(
    Token.Operator.EQ to null,
    Token.Operator.PLUSEQ to BinaryExpression.Type.ADD,
    Token.Operator.MINUSEQ to BinaryExpression.Type.SUB,
    Token.Operator.STAREQ to BinaryExpression.Type.MUL,
    Token.Operator.SLASHEQ to BinaryExpression.Type.DIV,
    Token.Operator.PERCENTEQ to BinaryExpression.Type.MOD,
    Token.Operator.DOTDOTEQ to BinaryExpression.Type.CONCAT,
)

sealed interface AssigmentResult {

    data class CompletedStatement(
        val assigmentStatement: AssigmentStatement
    ) : AssigmentResult

    data class OriginalExpression(
        val expression: Expression,
        val savedPosition: Int
    ) : AssigmentResult
}

fun Parser.Controller.statementAssigment(): AssigmentResult {
    val saved = position

    val method = supportAssignMethod { expression() }

    return when (method) {
        is AssignMethod.Decompose -> {
            consume(Token.Operator.EQ)
            val expression = expression()

            val entries = method.entries.map { entry ->
                val accessible = entry.key as? Accessible
                    ?: throw ParseException("Expression isn't accessible", data(saved))

                AssignMethod.Decompose.Entry(
                    value = accessible,
                    key = entry.key
                )
            }

            val result = AssigmentStatement(
                method = AssignMethod.Decompose(entries),
                expression = expression,
                binaryType = null,
                data = data(saved)
            )

            AssigmentResult.CompletedStatement(result)
        }

        is AssignMethod.Single -> if (match(*ASSIGN_OPERATORS.keys.toTypedArray())) {
            val type = get(-1)

            val expression = expression()

            val entry = method.entry as? Accessible ?: throw ParseException("Expression isn't accessible", data(saved))

            val result = AssigmentStatement(
                method = AssignMethod.Single(entry),
                expression = expression,
                binaryType = ASSIGN_OPERATORS[type],
                data = data(saved)
            )

            AssigmentResult.CompletedStatement(result)
        } else {
            AssigmentResult.OriginalExpression(
                expression = method.entry,
                savedPosition = saved
            )
        }
    }
}