package morphine.compiler.ast.node

data class FunctionExpression(
    val name: String?,
    val arguments: List<String>,
    val isRecursive: Boolean,
    val closures: List<Closure>,
    val statics: List<String>,
    val statement: Statement,
    override val data: Node.Data
) : Expression {

    data class Closure(
        val access: String,
        val alias: String
    )
}