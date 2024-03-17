package ru.unit.morphine.assembly.compiler.ast.node

data class VariableAccessible(
    val name: String,
    override val data: Node.Data
) : Accessible, Expression