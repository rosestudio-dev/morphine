package ru.unit.morphine.assembly.compiler.parser.exception

import ru.unit.morphine.assembly.compiler.ast.node.Node

class ParseException : RuntimeException {

    constructor(list: List<ParseException>) : super(
        list.mapIndexed { index, exception ->
            "$index. ${exception.message}"
        }.joinToString("\n")
    )

    constructor(
        text: String,
        data: Node.Data
    ) : super("[Line: ${data.lineData.line}] $text")
}