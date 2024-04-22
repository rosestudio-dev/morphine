package ru.unit.morphine.assembly.compiler.ast.node

data class VectorExpression(
    val elements: List<Expression>,
    override val data: Node.Data
) : Expression
