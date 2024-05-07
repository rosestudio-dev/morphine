package ru.unit.morphine.assembly.compiler.parser.functions

import ru.unit.morphine.assembly.compiler.ast.node.AssignMethod
import ru.unit.morphine.assembly.compiler.ast.node.DeclarationStatement
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.parser.Parser

fun Parser.Controller.statementFun(): Statement {
    val saved = position

    val function = expressionFunction(requireName = true)

    return DeclarationStatement(
        method = AssignMethod.Single(function.name!!),
        isMutable = false,
        expression = function,
        data = data(saved)
    )
}