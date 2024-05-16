package morphine.optimizer.tracer.functions

import morphine.bytecode.AbstractInstruction
import morphine.bytecode.Instruction
import morphine.optimizer.tracer.ControlFlowTree
import morphine.optimizer.tracer.Tracer

fun ControlFlowTree.traceValues() {
    val dataGen = DataGen()
    valuesCalc(dataGen)
}

private class DataGen {

    private var id = 0

    fun genData(): Tracer.TracedValue = Tracer.TracedValue.Data(id++)

    fun genConstant(constant: Int): Tracer.TracedValue = Tracer.TracedValue.Constant(
        index = id++,
        constant = constant
    )
}

private fun ControlFlowTree.valuesCalc(dataGen: DataGen) {
    val values = (0 until data.function.slotsCount).map {
        mutableMapOf(0 to dataGen.genData())
    }.toTypedArray()

    data.blocks.forEach { block ->
        block.nodes(originalData).forEach { node ->
            val destination = when (node.abstract) {
                is AbstractInstruction.Movement -> node.abstract.destination
                is AbstractInstruction.Processing -> node.abstract.destination
                is AbstractInstruction.Producer -> node.abstract.destination
                is AbstractInstruction.Consumer,
                is AbstractInstruction.Control -> null
            }

            if (destination != null) {
                val index = destination.value

                val data = when (val instruction = node.abstract.instruction) {
                    is Instruction.Load -> dataGen.genConstant(instruction.constant.value)
                    else -> dataGen.genData()
                }

                values[index][(node.tracedVersionsAfter[index] as Tracer.TracedVersion.Normal).version] = data
            }
        }
    }

    fun Tracer.TracedVersion.Phi.toValue(slot: Int): Tracer.TracedValue {
        val phi = versions.flatMap { version ->
            when (val value = values[slot][version]!!) {
                is Tracer.TracedValue.Constant -> listOf(value)
                is Tracer.TracedValue.Data -> listOf(value)
                is Tracer.TracedValue.Phi -> value.values
            }
        }.toSet()

        return Tracer.TracedValue.Phi(phi)
    }

    data.blocks.forEach { block ->
        block.nodes(originalData).forEach { node ->
            val instruction = node.abstract.instruction

            if (instruction is Instruction.Move) {
                val sourceSlot = instruction.source.value
                val destSlot = instruction.destination.value

                val data = when (val version = node.tracedVersionsBefore[sourceSlot]) {
                    is Tracer.TracedVersion.Normal -> values[sourceSlot][version.version]!!
                    is Tracer.TracedVersion.Phi -> version.toValue(sourceSlot)
                }

                values[destSlot][(node.tracedVersionsAfter[destSlot] as Tracer.TracedVersion.Normal).version] = data
            }
        }
    }

    data.blocks.forEach { block ->
        block.nodes(originalData).forEach { node ->
            node.tracedValuesBefore = node.tracedVersionsBefore.mapIndexed { slot, version ->
                when (version) {
                    is Tracer.TracedVersion.Normal -> values[slot][version.version]!!
                    is Tracer.TracedVersion.Phi -> version.toValue(slot)
                }
            }

            node.tracedValuesAfter = node.tracedVersionsAfter.mapIndexed { slot, version ->
                when (version) {
                    is Tracer.TracedVersion.Normal -> values[slot][version.version]!!
                    is Tracer.TracedVersion.Phi -> version.toValue(slot)
                }
            }
        }
    }
}
