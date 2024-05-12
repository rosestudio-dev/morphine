package morphine.compiler.ast.node

import morphine.bytecode.Argument

sealed interface Accessible : Expression {

    fun <T : Compiler> set(
        compiler: T,
        slot: Argument.Slot
    ) = compiler.set(accessible = this, slot = slot)
}