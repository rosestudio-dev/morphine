package morphine.compiler.ast.assembly.exception

import morphine.bytecode.LineData

data class CompilerException(
    override val message: String,
    val lineData: LineData? = null
) : Exception() {

    val messageWithLineData get() = "${lineData?.toString()?.plus(" ") ?: ""}${message}"
}