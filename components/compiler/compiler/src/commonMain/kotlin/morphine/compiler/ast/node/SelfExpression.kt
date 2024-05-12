package morphine.compiler.ast.node

data class SelfExpression(
    override val data: Node.Data
) : Expression