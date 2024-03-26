package ru.unit.morphine.assembly.compiler.ast.node

import ru.unit.morphine.assembly.bytecode.Argument

sealed interface Accessible : Expression {

    fun <T : Compiler> set(
        compiler: T,
        slot: Argument.Slot
    ) = compiler.set(accessible = this, slot = slot)
}