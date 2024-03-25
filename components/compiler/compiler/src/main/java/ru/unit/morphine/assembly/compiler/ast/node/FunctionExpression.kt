package ru.unit.morphine.assembly.compiler.ast.node

data class FunctionExpression(
    val name: String?,
    val arguments: List<String>,
    val statics: List<String>,
    val statement: Statement,
    override val data: Node.Data
) : Expression
