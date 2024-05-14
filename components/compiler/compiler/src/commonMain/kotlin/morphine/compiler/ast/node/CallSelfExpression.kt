package morphine.compiler.ast.node

data class CallSelfExpression(
    val self: Expression,
    val callable: Expression,
    val arguments: List<Expression>,
    val extractCallable: Boolean,
    override val data: Node.Data
) : Expression