package morphine.compiler.lexer.exception

import morphine.bytecode.LineData

class LexerException(
    text: String,
    lineData: LineData
) : Exception("$lineData $text")