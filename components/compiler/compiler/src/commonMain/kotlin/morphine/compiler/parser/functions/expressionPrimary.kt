package morphine.compiler.parser.functions

import morphine.compiler.lexer.Token
import morphine.compiler.parser.Parser

fun Parser.Controller.expressionPrimary() = when {
    look(Token.SystemWord.FUN) -> expressionFunction()
    look(Token.SystemWord.IF) -> expressionIf()
    match(Token.SystemWord.DO) -> expressionBlock(saved = position - 1)
    else -> expressionVariable()
}