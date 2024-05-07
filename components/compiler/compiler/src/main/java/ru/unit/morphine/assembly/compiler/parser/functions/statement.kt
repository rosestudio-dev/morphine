package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.CallExpression
import ru.unit.morphine.assembly.compiler.ast.node.CallSelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.EvalStatement
import ru.unit.morphine.assembly.compiler.ast.node.IncrementExpression
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

fun Parser.Controller.statement(
    allowSemicolon: Boolean = true,
    allowAllExpressions: Boolean = false
) = when {
    look(Token.SystemWord.YIELD) -> statementYield()
    !allowAllExpressions && look(Token.SystemWord.IF) -> statementIf()
    look(Token.SystemWord.ITERATOR) -> statementIterator()
    look(Token.SystemWord.WHILE) -> statementWhile()
    look(Token.SystemWord.DO) -> statementDoWhile(allowBlock = !allowAllExpressions)
    look(Token.SystemWord.FOR) -> statementFor()
    look(Token.SystemWord.BREAK) -> statementBreak()
    look(Token.SystemWord.CONTINUE) -> statementContinue()
    look(Token.SystemWord.RETURN) -> statementReturn()
    look(Token.SystemWord.LEAVE) -> statementLeave()
    look(Token.SystemWord.EVAL) -> statementEval()
    look(Token.SystemWord.PASS) -> statementEmpty()
    !allowAllExpressions && look(Token.SystemWord.FUN) -> statementFun()
    look(Token.SystemWord.VAL, Token.SystemWord.VAR) -> statementDeclaration()
    else -> {
        val saved = position
        when (val result = statementAssigment()) {
            is AssigmentResult.CompletedStatement -> result.assigmentStatement
            is AssigmentResult.OriginalExpression -> toExpression(
                result = result,
                allowAllExpressions = allowAllExpressions,
                saved = saved
            )
        }
    }
}.also {
    if (allowSemicolon) {
        match(Token.Operator.SEMICOLON)
    }
}

private fun Parser.Controller.toExpression(
    result: AssigmentResult.OriginalExpression,
    allowAllExpressions: Boolean,
    saved: Int
) = when {
    allowAllExpressions -> evalExpression(result)
    result.expression is CallExpression -> evalExpression(result)
    result.expression is CallSelfExpression -> evalExpression(result)
    result.expression is IncrementExpression -> evalExpression(result)

    else -> throw ParseException("Expected statement, but got expression", data(saved))
}

private fun Parser.Controller.evalExpression(
    result: AssigmentResult.OriginalExpression
) = EvalStatement(
    expression = result.expression,
    data = data(result.savedPosition)
)
