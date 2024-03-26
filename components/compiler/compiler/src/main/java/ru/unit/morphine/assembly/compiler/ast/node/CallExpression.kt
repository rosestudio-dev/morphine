package ru.unit.morphine.assembly.compiler.ast.node

data class CallExpression(
    val expression: Expression,
    val arguments: List<Expression>,
    override val data: Node.Data
) : Expression
