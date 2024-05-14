package morphine.bytecode

import morphine.utils.UID

data class Bytecode(
    val mainFunction: UID,
    val functions: List<Function>
) {

    data class Function(
        val uid: UID,
        val name: String,
        val instructions: List<Instruction>,
        val constants: List<Value>,
        val argumentsCount: Int,
        val staticsCount: Int,
        val closuresCount: Int,
        val slotsCount: Int,
        val paramsCount: Int,
        val optimize: Boolean
    )
}