package morphine.optimizer.strategy

import morphine.bytecode.AbstractInstruction
import morphine.bytecode.Argument
import morphine.optimizer.OptimizationStrategy
import morphine.optimizer.tracer.Tracer

class SlotZipperStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val slots = data.nodes.flatMap { wrapper ->
            when (wrapper.abstract) {
                is AbstractInstruction.Consumer -> wrapper.abstract.sources

                is AbstractInstruction.Movement -> listOf(
                    wrapper.abstract.source,
                    wrapper.abstract.destination,
                )

                is AbstractInstruction.Processing -> wrapper.abstract.sources + listOf(
                    wrapper.abstract.destination
                )

                is AbstractInstruction.Producer -> listOf(
                    wrapper.abstract.destination
                )

                is AbstractInstruction.Control -> emptyList()
            }
        }.distinct().sortedBy { slot -> slot.value }

        val result = data.nodes.map { wrapper ->
            val abstract = when (wrapper.abstract) {
                is AbstractInstruction.Consumer -> wrapper.abstract.copy(
                    sources = wrapper.abstract.sources.map { source ->
                        source.replace(slots)
                    }
                )

                is AbstractInstruction.Movement -> wrapper.abstract.copy(
                    source = wrapper.abstract.source.replace(slots),
                    destination = wrapper.abstract.destination.replace(slots)
                )

                is AbstractInstruction.Processing -> wrapper.abstract.copy(
                    sources = wrapper.abstract.sources.map { source ->
                        source.replace(slots)
                    },
                    destination = wrapper.abstract.destination.replace(slots)
                )

                is AbstractInstruction.Producer -> wrapper.abstract.copy(
                    destination = wrapper.abstract.destination.replace(slots)
                )

                is AbstractInstruction.Control -> null
            }

            if (abstract != null) {
                wrapper.copy(abstract = abstract)
            } else {
                wrapper
            }
        }

        return data.copy(
            function = data.function.copy(slotsCount = slots.size),
            nodes = result
        )
    }

    private fun Argument.Slot.replace(slots: List<Argument.Slot>): Argument.Slot {
        val newIndex = slots.indexOf(this)

        if (newIndex < 0) {
            throw SlotNotFoundException()
        }

        return Argument.Slot(newIndex)
    }

    class SlotNotFoundException : Exception()
}