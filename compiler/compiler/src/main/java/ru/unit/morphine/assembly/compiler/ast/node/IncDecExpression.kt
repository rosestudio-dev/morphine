package ru.unit.morphine.assembly.compiler.ast.node

data class IncDecExpression(
    val type: Type,
    val isPostfix: Boolean,
    val accessible: Accessible,
    override val data: Node.Data
) : Expression{

    enum class Type(val text: String) {
        INCREMENT("++"),
        DECREMENT("--"),
    }
}