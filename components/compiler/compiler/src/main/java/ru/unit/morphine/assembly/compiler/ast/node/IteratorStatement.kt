package ru.unit.morphine.assembly.compiler.ast.node

data class IteratorStatement(
    val method: AssignMethod<String>,
    val iterable: Expression,
    val statement: Statement,
    override val data: Node.Data
) : Statement