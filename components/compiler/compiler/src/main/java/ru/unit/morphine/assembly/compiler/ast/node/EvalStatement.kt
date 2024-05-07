package ru.unit.morphine.assembly.compiler.ast.node

data class EvalStatement(
    val expression: Expression,
    override val data: Node.Data
) : Statement