package morphine.compiler.ast.node

data class BreakStatement(
    override val data: Node.Data
) : Statement
