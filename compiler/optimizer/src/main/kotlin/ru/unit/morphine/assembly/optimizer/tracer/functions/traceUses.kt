package ru.unit.morphine.assembly.optimizer.tracer.functions

import ru.unit.morphine.assembly.bytecode.AbstractInstruction
import ru.unit.morphine.assembly.bytecode.Argument
import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.optimizer.tracer.ControlFlowTree
import ru.unit.morphine.assembly.optimizer.tracer.Tracer

fun ControlFlowTree.traceUses() {
    repeatUntilUnchanged(
        init = { init() },
        copy = { copy() },
        progress = { propagation() }
    )
}

private fun ControlFlowTree.init() = data.nodes.onEach { wrapper ->
    val sources = when (val abstract = wrapper.abstract) {
        is AbstractInstruction.Movement -> listOf(abstract.source)
        is AbstractInstruction.Consumer -> abstract.sources
        is AbstractInstruction.Processing -> abstract.sources
        is AbstractInstruction.Producer -> emptyList()
        is AbstractInstruction.Control -> emptyList()
    }.map { slot ->
        slot.value
    }

    wrapper.tracedUsesBefore = List(data.function.slotsCount) { index -> Tracer.TracedUse(index in sources) }
    wrapper.tracedUsesAfter = List(data.function.slotsCount) { Tracer.TracedUse(false) }
}

private fun ControlFlowTree.propagation(): List<Tracer.Node> {
    return data.nodes.onEach { wrapper ->
        wrapper.destinations.forEach { destination ->
            val dest = data.nodes[destination]

            dest.tracedUsesAfter = dest.tracedUsesAfter.mapIndexed { index, useAfter ->
                val useBefore = wrapper.tracedUsesBefore[index]

                if(useAfter.used) {
                    useAfter
                } else {
                    useBefore
                }
            }

            val (sources, destinationSlot) = when (val abstract = dest.abstract) {
                is AbstractInstruction.Movement -> listOf(abstract.source) to abstract.destination
                is AbstractInstruction.Processing -> abstract.sources to abstract.destination
                is AbstractInstruction.Producer -> emptyList<Argument.Slot>() to abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control -> emptyList<Argument.Slot>() to null
            }

            dest.tracedUsesBefore = dest.tracedUsesBefore.mapIndexed { index, useBefore ->
                val useAfter = dest.tracedUsesAfter[index]

                if(useBefore.used) {
                    useBefore
                } else if(destinationSlot?.value == index) {
                    Tracer.TracedUse(sources.any { source -> source.value == index })
                } else {
                    useAfter
                }
            }
        }
    }
}
