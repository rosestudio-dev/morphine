package ru.unit.morphine.assembly.compiler.lexer

import ru.unit.morphine.assembly.bytecode.LineData

data class LinedToken(
    val token: Token,
    val lineData: LineData
)
