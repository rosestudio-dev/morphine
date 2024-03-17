package ru.unit.morphine.assembly.compiler.ast.compiler.exception

import ru.unit.morphine.assembly.bytecode.LineData

data class CompilerException(
    override val message: String,
    val lineData: LineData? = null
) : Exception()