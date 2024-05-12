package morphine.compiler.ast.node

data class ForStatement(
    val initial: Statement,
    val condition: Expression,
    val iterator: Statement,
    val statement: Statement,
    override val data: Node.Data
) : Statement