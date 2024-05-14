package morphine.compiler.ast.node

data class BlockExpression(
    val statements: List<Statement>,
    val expression: Expression,
    override val data: Node.Data
) : Expression