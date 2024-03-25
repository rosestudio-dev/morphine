package ru.unit.morphine.assembly.compiler.ast.node

data class UnaryExpression(
    val type: Type,
    val expression: Expression,
    override val data: Node.Data
) : Expression {

    enum class Type {
        NEGATE,
        NOT,
        TYPE,
        LEN,
        REF,
        DEREF,
    }
}