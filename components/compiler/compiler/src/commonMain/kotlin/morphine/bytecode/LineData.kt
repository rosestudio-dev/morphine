package morphine.bytecode

data class LineData(
    val line: Int,
    val column: Int,
) {

    override fun toString() = "[$line:$column]"
}
