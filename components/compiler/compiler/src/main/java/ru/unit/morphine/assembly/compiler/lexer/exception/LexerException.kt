package ru.unit.morphine.assembly.compiler.lexer.exception

import ru.unit.morphine.assembly.bytecode.LineData

class LexerException(
    text: String,
    lineData: LineData
) : RuntimeException("$lineData $text")