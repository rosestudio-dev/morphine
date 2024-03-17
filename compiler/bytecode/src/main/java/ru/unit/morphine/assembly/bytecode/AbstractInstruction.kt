package ru.unit.morphine.assembly.bytecode

sealed interface AbstractInstruction {

    val instruction: Instruction

    data class Control(
        override val instruction: Instruction,
    ) : AbstractInstruction

    data class Processing(
        override val instruction: Instruction,
        val sources: List<Argument.Slot>,
        val destination: Argument.Slot
    ) : AbstractInstruction

    data class Consumer(
        override val instruction: Instruction,
        val sources: List<Argument.Slot>,
    ) : AbstractInstruction

    data class Producer(
        override val instruction: Instruction,
        val destination: Argument.Slot,
    ) : AbstractInstruction

    data class Movement(
        override val instruction: Instruction,
        val source: Argument.Slot,
        val destination: Argument.Slot,
    ) : AbstractInstruction
}