package morphine.optimizer.strategy

import morphine.bytecode.AbstractInstruction
import morphine.bytecode.Argument
import morphine.optimizer.OptimizationStrategy
import morphine.optimizer.tracer.Tracer

class MeaninglessStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val candidates = data.nodes.mapNotNull { wrapper ->
            val destination = when (wrapper.abstract) {
                is AbstractInstruction.Movement -> wrapper.abstract.destination
                is AbstractInstruction.Producer -> wrapper.abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control,
                is AbstractInstruction.Processing -> null
            } ?: return@mapNotNull null

            Candidate(
                destination = destination,
                destinationVersion = wrapper.getDestVersion(destination),
                wrapper = wrapper
            )
        }

        data.nodes.fold(candidates) { acc, wrapper ->
            when (wrapper.abstract) {
                is AbstractInstruction.Movement -> acc.filterNot { candidate ->
                    val source = wrapper.abstract.source
                    val isEqual = source == candidate.destination
                    val versionIsEqual =
                        candidate.destinationVersion.versionUsedIn(wrapper.getSourceVersion(source))

                    isEqual && versionIsEqual
                }

                is AbstractInstruction.Consumer -> acc.filterNot { candidate ->
                    wrapper.abstract.sources.any { source ->
                        val isEqual = source == candidate.destination
                        val versionIsEqual =
                            candidate.destinationVersion.versionUsedIn(wrapper.getSourceVersion(source))

                        isEqual && versionIsEqual
                    }
                }

                is AbstractInstruction.Processing -> acc.filterNot { candidate ->
                    wrapper.abstract.sources.any { source ->
                        val isEqual = source == candidate.destination
                        val versionIsEqual =
                            candidate.destinationVersion.versionUsedIn(wrapper.getSourceVersion(source))

                        isEqual && versionIsEqual
                    }
                }

                is AbstractInstruction.Producer,
                is AbstractInstruction.Control -> acc
            }
        }.forEach { candidate ->
            candidate.wrapper.isAvailable = false
        }

        return data
    }

    private fun Tracer.TracedVersion.versionUsedIn(tracedVersion: Tracer.TracedVersion?): Boolean {
        val normal = (this as Tracer.TracedVersion.Normal)
        return when (tracedVersion) {
            null -> false
            is Tracer.TracedVersion.Normal -> normal.version == tracedVersion.version
            is Tracer.TracedVersion.Phi -> normal.version in tracedVersion.versions
        }
    }

    private data class Candidate(
        val destination: Argument.Slot,
        val destinationVersion: Tracer.TracedVersion,
        val wrapper: Tracer.Node
    )
}