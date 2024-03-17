package ru.unit.morphine.assembly.compiler.ast.node

import ru.unit.morphine.assembly.bytecode.LineData

sealed interface Node {

    val data: Data

    fun accept(visitor: Visitor) = visitor.visit(this)

    data class Data(
        val lineData: LineData
    )
}