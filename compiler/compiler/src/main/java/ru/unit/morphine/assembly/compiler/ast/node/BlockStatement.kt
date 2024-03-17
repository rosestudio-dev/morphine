package ru.unit.morphine.assembly.compiler.ast.node

data class BlockStatement(
    val statements: List<Statement>,
    override val data: Node.Data
) : Statement, Expression