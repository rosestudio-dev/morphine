package ru.unit.morphine.assembly.compiler.ast.node

data class DoWhileStatement(
    val condition: Expression,
    val statement: Statement,
    override val data: Node.Data
) : Statement