package ru.unit.morphine.assembly.optimizer.strategy

import ru.unit.morphine.assembly.bytecode.Instruction
import ru.unit.morphine.assembly.bytecode.Value
import ru.unit.morphine.assembly.optimizer.OptimizationStrategy
import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import ru.unit.morphine.assembly.bytecode.generated.abstract

class LinearBranchStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val result = data.nodes.map { wrapper ->
            val instruction = wrapper.abstract.instruction

            if (instruction !is Instruction.JumpIf) {
                return@map wrapper
            }

            val traced = wrapper.getSourceValue(instruction.source)

            if (traced is Tracer.TracedValue.Constant) {
                val constant = data.function.constants[traced.index]

                return@map if (constant is Value.Boolean) {
                    val position = if (constant.value) {
                        instruction.ifPosition
                    } else {
                        instruction.elsePosition
                    }

                    wrapper.copy(
                        abstract = Instruction.Jump(
                            position = position,
                            lineData = instruction.lineData
                        ).abstract()
                    )
                } else {
                    wrapper
                }
            }

            if (instruction.ifPosition == instruction.elsePosition) {
                return@map wrapper.copy(
                    abstract = Instruction.Jump(
                        position = instruction.ifPosition,
                        lineData = instruction.lineData
                    ).abstract()
                )
            }

            return@map wrapper
        }

        return data.copy(nodes = result)
    }
}