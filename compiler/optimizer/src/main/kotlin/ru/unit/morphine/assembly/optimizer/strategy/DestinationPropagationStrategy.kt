package ru.unit.morphine.assembly.optimizer.strategy

import ru.unit.morphine.assembly.bytecode.AbstractInstruction
import ru.unit.morphine.assembly.bytecode.Argument
import ru.unit.morphine.assembly.optimizer.OptimizationStrategy
import ru.unit.morphine.assembly.optimizer.tracer.Tracer

class DestinationPropagationStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val propagations = mutableListOf<Propagation>()

        val instructions = data.nodes.map { wrapper ->
            val destination = when (wrapper.abstract) {
                is AbstractInstruction.Processing -> wrapper.abstract.destination
                is AbstractInstruction.Producer -> wrapper.abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Movement,
                is AbstractInstruction.Control -> return@map wrapper
            }

            val traced = wrapper.getDestValue(destination)

            val tracedData = destination.trace(
                traced = traced,
                wrappers = data.nodes
            ) ?: return@map wrapper

            propagations.add(
                Propagation(
                    traced = tracedData.traced,
                    destination = tracedData.slot,
                    tracedFrom = traced,
                    from = destination,
                    index = tracedData.index
                )
            )

            val new = when (wrapper.abstract) {
                is AbstractInstruction.Processing -> wrapper.abstract.copy(
                    destination = tracedData.slot
                )

                is AbstractInstruction.Producer -> wrapper.abstract.copy(
                    destination = tracedData.slot
                )

                is AbstractInstruction.Movement,
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control -> return@map wrapper
            }

            wrapper.copy(abstract = new)
        }

        val result = instructions.map { wrapper ->
            val abstract = when (wrapper.abstract) {
                is AbstractInstruction.Movement -> wrapper.abstract.copy(
                    source = wrapper.abstract.source.replace(wrapper, propagations)
                )

                is AbstractInstruction.Consumer -> wrapper.abstract.copy(
                    sources = wrapper.abstract.sources.map { source ->
                        source.replace(wrapper, propagations)
                    }
                )

                is AbstractInstruction.Processing -> wrapper.abstract.copy(
                    sources = wrapper.abstract.sources.map { source ->
                        source.replace(wrapper, propagations)
                    }
                )

                is AbstractInstruction.Producer,
                is AbstractInstruction.Control -> null
            }

            if (abstract != null) {
                wrapper.copy(abstract = abstract)
            } else {
                wrapper
            }
        }

        propagations.forEach { propagation ->
            result[propagation.index].isAvailable = false
        }

        return data.copy(nodes = result)
    }

    private fun Argument.Slot.trace(
        traced: Tracer.TracedValue,
        wrappers: List<Tracer.Node>
    ): TracedData? {
        val movement = wrappers.withIndex().singleOrNull { (_, wrapper) ->
            when (wrapper.abstract) {
                is AbstractInstruction.Movement -> {
                    val tracedIsEqual = wrapper.getSourceValue(wrapper.abstract.source) == traced
                    val isEqual = wrapper.abstract.source == this

                    tracedIsEqual && isEqual
                }

                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control,
                is AbstractInstruction.Processing,
                is AbstractInstruction.Producer -> false
            }
        } ?: return null

        val usages = wrappers.count { wrapper ->
            val sources = when (wrapper.abstract) {
                is AbstractInstruction.Movement -> listOf(wrapper.abstract.source)
                is AbstractInstruction.Processing -> wrapper.abstract.sources
                is AbstractInstruction.Consumer -> wrapper.abstract.sources
                is AbstractInstruction.Producer,
                is AbstractInstruction.Control -> return@count false
            }

            sources.any { source ->
                wrapper.getSourceValue(source) == traced && source == this
            }
        }

        if (usages > 1) {
            return null
        }

        val abstract = movement.value.abstract as AbstractInstruction.Movement

        return TracedData(
            slot = abstract.destination,
            traced = movement.value.getDestValue(abstract.destination),
            index = movement.index
        )
    }

    private fun Argument.Slot.replace(
        wrapper: Tracer.Node,
        propagations: List<Propagation>
    ) = propagations.find { propagation ->
        val isEqual = this == propagation.from

        val tracedValueIsEqual =
            wrapper.getSourceValue(this) == propagation.tracedFrom

        val destinationValueIsEqual =
            wrapper.getSourceValue(propagation.destination) == propagation.traced

        isEqual && tracedValueIsEqual && destinationValueIsEqual
    }?.destination ?: this

    private data class Propagation(
        val traced: Tracer.TracedValue,
        val destination: Argument.Slot,
        val tracedFrom: Tracer.TracedValue,
        val from: Argument.Slot,
        val index: Int,
    )

    private data class TracedData(
        val slot: Argument.Slot,
        val traced: Tracer.TracedValue,
        val index: Int,
    )
}