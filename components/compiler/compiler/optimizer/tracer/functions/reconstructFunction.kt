package morphine.optimizer.tracer.functions

import morphine.bytecode.Argument
import morphine.bytecode.Instruction
import morphine.bytecode.Bytecode
import morphine.optimizer.tracer.Tracer
import morphine.bytecode.generated.normalize
import kotlin.math.min

fun Tracer.Data.reconstructFunction(): Bytecode.Function {
    val availableCount = nodes.count(Tracer.Node::isAvailable)
    val indices = mutableListOf<Int>()

    nodes.fold(0) { last, instruction ->
        indices.add(last)

        if (instruction.isAvailable) {
            last + 1
        } else {
            last
        }
    }

    val instructions = nodes.mapNotNull { instructionWrapper ->
        val convertedInstruction = when (val instruction = instructionWrapper.abstract.normalize()) {
            is Instruction.Jump -> {
                val newPosition = findAvailableInstruction(instruction.position, availableCount, indices)

                Instruction.Jump(
                    position = newPosition,
                    lineData = instruction.lineData
                )
            }

            is Instruction.JumpIf -> {
                val newIfPosition = findAvailableInstruction(instruction.ifPosition, availableCount, indices)
                val newElsePosition = findAvailableInstruction(instruction.elsePosition, availableCount, indices)

                Instruction.JumpIf(
                    source = instruction.source,
                    ifPosition = newIfPosition,
                    elsePosition = newElsePosition,
                    lineData = instruction.lineData
                )
            }

            else -> instruction
        }

        if (instructionWrapper.isAvailable) {
            convertedInstruction
        } else {
            null
        }
    }

    return function.copy(
        instructions = instructions
    )
}

private fun Tracer.Data.findAvailableInstruction(position: Argument.Position, availableCount: Int, indices: List<Int>): Argument.Position {
    val pos = nodes.indexOfFirst { node ->
        node.index == position.value
    }

    if (pos < 0) {
        return Argument.Position(availableCount)
    }

    val found = (pos until nodes.size).find { index ->
        nodes[index].isAvailable
    }?.let { index ->
        indices[index]
    } ?: nodes.size

    val newPosition = min(found, availableCount)

    return Argument.Position(newPosition)
}