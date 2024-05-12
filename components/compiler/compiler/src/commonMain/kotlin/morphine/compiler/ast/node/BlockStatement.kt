package morphine.compiler.ast.node

data class BlockStatement(
    val statements: List<Statement>,
    override val data: Node.Data
) : Statement