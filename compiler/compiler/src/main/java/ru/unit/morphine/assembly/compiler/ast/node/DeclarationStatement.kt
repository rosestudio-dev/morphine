package ru.unit.morphine.assembly.compiler.ast.node

data class DeclarationStatement(
    val isMutable: Boolean,
    val names: List<String>,
    val expressions: List<Expression>,
    override val data: Node.Data
) : Statement
