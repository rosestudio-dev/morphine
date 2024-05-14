package morphine.compiler.ast.node

data class TableExpression(
    val elements: List<Element>,
    override val data: Node.Data
) : Expression {

    data class Element(
        val key: Expression,
        val value: Expression,
    )
}
