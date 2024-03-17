package ru.unit.morphine.assembly.optimizer.strategy

import ru.unit.morphine.assembly.bytecode.Argument
import ru.unit.morphine.assembly.bytecode.Instruction
import ru.unit.morphine.assembly.optimizer.OptimizationStrategy
import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import ru.unit.morphine.assembly.bytecode.generated.abstract

class PositionPropagationStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val result = data.nodes.map { wrapper ->
            val instruction = when (val instruction = wrapper.abstract.instruction) {
                is Instruction.Jump -> instruction.copy(
                    position = instruction.position.trace(data.nodes)
                )

                is Instruction.JumpIf -> instruction.copy(
                    ifPosition = instruction.ifPosition.trace(data.nodes),
                    elsePosition = instruction.elsePosition.trace(data.nodes)
                )

                else -> instruction
            }

            wrapper.copy(
                abstract = instruction.abstract()
            )
        }

        return data.copy(nodes = result)
    }

    private fun Argument.Position.trace(
        nodes: List<Tracer.Node>,
    ): Argument.Position {
        val instruction = nodes.find { node -> node.index == value }?.abstract?.instruction

        return if (instruction is Instruction.Jump) {
            instruction.position
        } else {
            this
        }
    }
}