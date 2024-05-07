package ru.unit.morphine.assembly.compiler.ast.node

data class IfExpression(
    val condition: Expression,
    val ifExpression: Expression,
    val elseExpression: Expression,
    override val data: Node.Data
) : Expression