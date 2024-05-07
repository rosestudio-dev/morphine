package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.expression(): Expression {
    return expressionOr()
}