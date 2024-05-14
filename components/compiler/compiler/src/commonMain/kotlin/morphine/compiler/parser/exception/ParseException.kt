package morphine.compiler.parser.exception

import morphine.compiler.ast.node.Node

class ParseException : Exception {

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