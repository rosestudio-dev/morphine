package morphine.optimizer.strategy

import morphine.bytecode.Argument
import morphine.bytecode.Instruction
import morphine.optimizer.OptimizationStrategy
import morphine.optimizer.tracer.Tracer
import morphine.bytecode.generated.abstract

class ConstantZipperStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val constants = data.nodes.mapNotNull { wrapper ->
            val instruction = wrapper.abstract.instruction
            if (instruction is Instruction.Load) {
                data.function.constants[instruction.constant.value]
            } else {
                null
            }
        }.distinct()

        val result = data.nodes.map { wrapper ->
            val instruction = wrapper.abstract.instruction
            if (instruction is Instruction.Load) {
                val oldValue = data.function.constants[instruction.constant.value]
                val newIndex = constants.indexOf(oldValue)

                if (newIndex < 0) {
                    throw ConstantNotFoundException()
                }

                val load = instruction.copy(
                    constant = Argument.Constant(newIndex)
                )

                wrapper.copy(
                    abstract = load.abstract()
                )
            } else {
                wrapper
            }
        }

        return data.copy(
            nodes = result,
            function = data.function.copy(
                constants = constants
            )
        )
    }

    class ConstantNotFoundException : Exception()
}