package morphine.compiler.ast.node

import morphine.bytecode.Value

data class ValueExpression(
    val value: Value,
    override val data: Node.Data
) : Expression