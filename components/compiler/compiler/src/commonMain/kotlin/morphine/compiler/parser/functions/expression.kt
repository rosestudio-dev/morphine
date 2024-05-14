package morphine.compiler.parser.functions

import morphine.compiler.ast.node.Expression
import morphine.compiler.parser.Parser

fun Parser.Controller.expression(): Expression {
    return expressionOr()
}