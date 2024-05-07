package ru.unit.morphine.assembly.compiler.ast.compilerold.exception

import ru.unit.morphine.assembly.bytecode.LineData

data class CompilerException(
    override val message: String,
    val lineData: LineData? = null
) : Exception() {

    val messageWithLineData get() = "${lineData?.toString()?.plus(" ") ?: ""}${message}"
}