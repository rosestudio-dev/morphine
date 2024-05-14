package morphine.optimizer.tracer.functions

import morphine.bytecode.Bytecode
import morphine.bytecode.Instruction
import morphine.optimizer.tracer.Tracer
import morphine.bytecode.generated.abstract

fun Bytecode.Function.toData() = Tracer.Data(
    function = this,
    nodes = instructions.mapIndexed { index, instruction ->
        Tracer.Node(
            index = index,
            abstract = instruction.abstract(),
            tracedVersionsBefore = emptyList(),
            tracedVersionsAfter = emptyList(),
            tracedValuesBefore = emptyList(),
            tracedValuesAfter = emptyList(),
            destinations = emptySet(),
        )
    },
    blocks = run {
        val positions = mutableSetOf(0, instructions.size)

        instructions.forEachIndexed { index, instruction ->
            val list = when (instruction) {
                is Instruction.Jump -> listOf(index + 1, instruction.position.value)
                is Instruction.JumpIf -> listOf(index + 1, instruction.ifPosition.value, instruction.elsePosition.value)
                is Instruction.Leave -> listOf(index + 1)
                else -> emptyList()
            }

            positions.addAll(list)
        }

        val sortedPositions = positions.filter { pos ->
            pos <= instructions.size
        }.sorted()

        (0 until (sortedPositions.size - 1)).map { index ->
            Tracer.Block(
                id = index,
                start = sortedPositions[index],
                end = sortedPositions[index + 1]
            )
        }
    }
)