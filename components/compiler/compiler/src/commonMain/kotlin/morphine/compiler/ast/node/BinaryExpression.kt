package morphine.compiler.ast.node

data class BinaryExpression(
    val type: Type,
    val expressionA: Expression,
    val expressionB: Expression,
    override val data: Node.Data
) : Expression {

    enum class Type {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        EQUALS,
        LESS,
        LESS_EQUALS,
        OR,
        AND,
        CONCAT,
    }
}