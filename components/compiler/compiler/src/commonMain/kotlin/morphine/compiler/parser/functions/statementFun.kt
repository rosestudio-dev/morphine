package morphine.compiler.parser.functions

import morphine.compiler.ast.node.AssignMethod
import morphine.compiler.ast.node.DeclarationStatement
import morphine.compiler.ast.node.Statement
import morphine.compiler.parser.Parser

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