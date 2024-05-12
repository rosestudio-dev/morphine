package morphine.compiler.ast.node

data class ReturnStatement(
    val expression: Expression,
    override val data: Node.Data
) : Statement