package ru.unit.morphine.assembly.compiler.ast.node

data class AssigmentStatement(
    val method: AssignMethod<Accessible>,
    val expression: Expression,
    val binaryType: BinaryExpression.Type?,
    override val data: Node.Data
) : Statement