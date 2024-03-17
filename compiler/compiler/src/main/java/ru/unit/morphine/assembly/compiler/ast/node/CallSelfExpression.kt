package ru.unit.morphine.assembly.compiler.ast.node

data class CallSelfExpression(
    val callable: Expression,
    val access: Expression,
    val arguments: List<Expression>,
    override val data: Node.Data
) : Expression
