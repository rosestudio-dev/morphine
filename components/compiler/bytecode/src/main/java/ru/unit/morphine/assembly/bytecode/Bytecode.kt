package ru.unit.morphine.assembly.bytecode

import java.util.*

data class Bytecode(
    val mainFunction: UUID,
    val functions: List<Function>
) {

    data class Function(
        val uuid: UUID,
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