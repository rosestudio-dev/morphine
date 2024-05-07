package ru.unit.morphine.assembly.compiler.ast.node

data class IncrementExpression(
    val type: Type,
    val isPostfix: Boolean,
    val accessible: Accessible,
    override val data: Node.Data
) : Expression {

    enum class Type {
        INCREMENT,
        DECREMENT,
    }
}