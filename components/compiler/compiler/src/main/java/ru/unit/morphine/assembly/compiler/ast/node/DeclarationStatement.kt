package ru.unit.morphine.assembly.compiler.ast.node

data class DeclarationStatement(
    val isMutable: Boolean,
    val method: AssignMethod<String>,
    val expression: Expression,
    override val data: Node.Data
) : Statement