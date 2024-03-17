package ru.unit.morphine.assembly.compiler.ast.node

data class SelfExpression(
    override val data: Node.Data
) : Expression
