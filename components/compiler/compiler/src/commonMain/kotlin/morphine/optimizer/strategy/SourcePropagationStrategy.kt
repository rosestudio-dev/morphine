package morphine.optimizer.strategy

import morphine.bytecode.AbstractInstruction
import morphine.bytecode.Argument
import morphine.optimizer.OptimizationStrategy
import morphine.optimizer.tracer.Tracer

class SourcePropagationStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val propagations = data.nodes.mapNotNull { wrapper ->
            if (wrapper.abstract is AbstractInstruction.Movement) {
                val source = wrapper.abstract.source

                Propagation(
                    source = source,
                    traced = wrapper.getSourceValue(source)
                )
            } else {
                null
            }
        }

        val result = data.nodes.map { wrapper ->
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

        return data.copy(nodes = result)
    }

    private fun Argument.Slot.replace(
        wrapper: Tracer.Node,
        propagations: List<Propagation>
    ) = propagations.find { propagation ->
        val tracedVersionIsEqual = wrapper.getSourceValue(this) == propagation.traced
        val sourceVersionIsEqual = wrapper.getSourceValue(propagation.source) == propagation.traced

        sourceVersionIsEqual && tracedVersionIsEqual
    }?.source ?: this

    private data class Propagation(
        val source: Argument.Slot,
        val traced: Tracer.TracedValue,
    )
}