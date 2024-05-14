package morphine.compiler.parser.functions

import morphine.compiler.ast.node.EvalStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.statementEval(): Statement {
    val saved = position
    consume(Token.SystemWord.EVAL)
    return EvalStatement(expression(), data(saved))
}