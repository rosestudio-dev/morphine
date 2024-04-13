package ru.unit.morphine.assembly.optimizer.tracer.functions

import org.jgrapht.traverse.BreadthFirstIterator
import ru.unit.morphine.assembly.bytecode.AbstractInstruction
import ru.unit.morphine.assembly.optimizer.exception.OptimizerException
import ru.unit.morphine.assembly.optimizer.tracer.ControlFlowTree
import ru.unit.morphine.assembly.optimizer.tracer.Tracer

fun ControlFlowTree.traceVersions() {
    valuesCalc()
    sourceCalc()
}

private fun ControlFlowTree.valuesCalc() {
    val slotVersions = MutableList(data.function.slotsCount) { 0 }

    val slots = MutableList<Tracer.TracedVersion>(data.function.slotsCount) {
        Tracer.TracedVersion.Normal(0)
    }

    data.blocks.forEach { block ->
        block.nodes(originalData).forEach { node ->
            val destination = when (val abstract = node.abstract) {
                is AbstractInstruction.Processing -> abstract.destination
                is AbstractInstruction.Producer -> abstract.destination
                is AbstractInstruction.Movement -> abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control -> null
            }

            node.tracedVersionsBefore = slots.toList()

            if (destination != null) {
                slotVersions[destination.value]++
                slots[destination.value] = Tracer.TracedVersion.Normal(slotVersions[destination.value])
            }

            node.tracedVersionsAfter = slots.toList()
        }
    }
}

private fun ControlFlowTree.extractCalc(): Map<Tracer.Block, Set<Tracer.Block>> {
    val iter = BreadthFirstIterator(dominatorTree.iDomGraph).apply {
        isCrossComponentTraversal = false
    }

    val blocks = data.blocks.associateWith { block ->
        if (dominatorTree.root == block) {
            null
        } else {
            dominatorTree.getIDom(block)
        } to mutableSetOf<Tracer.Block>()
    }

    while (iter.hasNext()) {
        val block = iter.next()

        val fronts = dominatorTree.getFronts(block)
        fronts.forEach { front ->
            blocks[front]!!.second.add(block)
        }
    }

    return blocks.mapValues { (_, pair) ->
        pair.second + (pair.first?.let(::setOf) ?: emptySet())
    }
}

private fun ControlFlowTree.sourceCalc() {
    val extractBlocks = extractCalc()

    val iter = BreadthFirstIterator(dominatorTree.iDomGraph).apply {
        isCrossComponentTraversal = false
    }

    while (iter.hasNext()) {
        val block = iter.next()

        val variants = extractBlocks[block]!!.map { front ->
            front.nodes(originalData).last().tracedVersionsAfter
        }.ifEmpty {
            listOf(
                List<Tracer.TracedVersion>(data.function.slotsCount) {
                    Tracer.TracedVersion.Normal(0)
                }
            )
        }

        val slots = (0 until data.function.slotsCount).map { slot ->
            val versions = variants.map { list ->
                list[slot]
            }.flatMap { version ->
                when (version) {
                    is Tracer.TracedVersion.Normal -> listOf(version.version)
                    is Tracer.TracedVersion.Phi -> version.versions.toList()
                }
            }.sorted().toSet()

            when (versions.size) {
                0 -> throw OptimizerException("Unknown slot version")
                1 -> Tracer.TracedVersion.Normal(versions.single())
                else -> Tracer.TracedVersion.Phi(versions)
            }
        }.toMutableList()

        val nodes = block.nodes(originalData)

        nodes.forEach { node ->
            val destination = when (val abstract = node.abstract) {
                is AbstractInstruction.Processing -> abstract.destination
                is AbstractInstruction.Producer -> abstract.destination
                is AbstractInstruction.Movement -> abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control -> null
            }

            node.tracedVersionsBefore = slots.toList()

            if (destination != null) {
                slots[destination.value] = node.tracedVersionsAfter[destination.value]
            }

            node.tracedVersionsAfter = slots.toList()
        }
    }
}
