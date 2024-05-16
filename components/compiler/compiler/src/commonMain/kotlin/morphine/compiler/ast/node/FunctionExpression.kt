package morphine.compiler.ast.node

data class FunctionExpression(
    val name: String?,
    val arguments: List<String>,
    val isRecursive: Boolean,
    val closureMode: ClosureMode,
    val statics: List<String>,
    val statement: Statement,
    override val data: Node.Data
) : Expression {

    sealed interface ClosureMode {

        data object Automatic : ClosureMode

        data class Manual(
            val list: List<Closure> = emptyList()
        ) : ClosureMode
    }

    data class Closure(
        val access: String,
        val alias: String
    )
}