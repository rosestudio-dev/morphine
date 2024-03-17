package ru.unit.morphine.assembly.optimizer.tracer.functions

import ru.unit.morphine.assembly.bytecode.Instruction
import ru.unit.morphine.assembly.optimizer.tracer.ControlFlowTree
import ru.unit.morphine.assembly.optimizer.tracer.Tracer

fun ControlFlowTree.traceDestinations() {
    val destinations = List(data.nodes.size) {
        mutableSetOf<Int>()
    }

    data.nodes.forEachIndexed { index, wrapper ->
        when (val instruction = wrapper.abstract.instruction) {
            is Instruction.Jump -> {
                destinations.getOrNull(instruction.position.value)?.add(index)
            }

            is Instruction.JumpIf -> {
                destinations.getOrNull(instruction.ifPosition.value)?.add(index)
                destinations.getOrNull(instruction.elsePosition.value)?.add(index)
            }

            else -> Unit
        }

        val hasPrev = when (data.nodes.getOrNull(index - 1)?.abstract?.instruction) {
            null,
            is Instruction.Jump,
            is Instruction.JumpIf,
            is Instruction.Leave -> false

            else -> true
        }

        if (hasPrev) {
            destinations[index].add(index - 1)
        }
    }

    data.nodes.forEachIndexed { index, wrapper ->
        wrapper.destinations = destinations[index]
    }
}