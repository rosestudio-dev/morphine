package morphine.compiler.lexer

import morphine.bytecode.LineData

data class LinedToken(
    val token: Token,
    val lineData: LineData
)
