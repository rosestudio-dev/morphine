package morphine.compiler.ast.node

import morphine.bytecode.LineData

sealed interface Node {

    val data: Data

    fun accept(visitor: Visitor) = visitor.visit(this)

    data class Data(
        val lineData: LineData
    )
}