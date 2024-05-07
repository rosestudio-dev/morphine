package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expressionPrimary() = when {
    look(Token.SystemWord.FUN) -> expressionFunction()
    look(Token.SystemWord.IF) -> expressionIf()
    match(Token.SystemWord.DO) -> expressionBlock(saved = position - 1)
    else -> expressionVariable()
}