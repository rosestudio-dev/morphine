package ru.unit.morphine.assembly.compiler.ast.node

data class IfStatement(
    val condition: Expression,
    val ifStatement: Statement,
    val elseStatement: Statement,
    override val data: Node.Data
) : Statement, Expression