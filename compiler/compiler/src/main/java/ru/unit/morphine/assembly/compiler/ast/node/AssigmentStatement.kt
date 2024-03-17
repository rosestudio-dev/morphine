package ru.unit.morphine.assembly.compiler.ast.node

data class AssigmentStatement(
    val accessibles: List<Accessible>,
    val expressions: List<Expression>,
    val binaryType: BinaryExpression.Type?,
    override val data: Node.Data
) : Statement