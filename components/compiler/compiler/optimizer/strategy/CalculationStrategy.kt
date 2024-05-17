package morphine.optimizer.strategy

import morphine.bytecode.AbstractInstruction
import morphine.bytecode.Argument
import morphine.bytecode.Instruction
import morphine.bytecode.LineData
import morphine.bytecode.Opcode
import morphine.bytecode.Value
import morphine.bytecode.generated.abstract
import morphine.optimizer.OptimizationStrategy
import morphine.optimizer.tracer.Tracer

class CalculationStrategy : OptimizationStrategy {

    override fun optimize(data: Tracer.Data): Tracer.Data {
        val constants = data.function.constants.toMutableList()

        val result = data.nodes.map { wrapper ->
            when (val abstract = wrapper.abstract) {
                is AbstractInstruction.Processing -> wrapper.copy(
                    abstract = abstract.process(wrapper, constants)
                )

                else -> wrapper
            }
        }

        return data.copy(
            function = data.function.copy(constants = constants),
            nodes = result
        )
    }

    private fun AbstractInstruction.Processing.process(
        wrapper: Tracer.Node,
        constants: MutableList<Value>
    ) = when (instruction.opcode) {
        Opcode.EQUAL -> equal(wrapper, constants)

        Opcode.ADD,
        Opcode.SUB,
        Opcode.MUL,
        Opcode.DIV,
        Opcode.MOD,
        Opcode.LESS,
        Opcode.LESS_EQUAL,
        Opcode.AND,
        Opcode.OR,
        Opcode.CONCAT -> binary(wrapper, constants)

        Opcode.NEGATIVE,
        Opcode.NOT,
        Opcode.LENGTH -> unary(wrapper, constants)

        else -> this
    }

    private fun AbstractInstruction.Processing.unary(
        wrapper: Tracer.Node,
        constants: MutableList<Value>,
    ): AbstractInstruction {
        val valueA = when (val traced = wrapper.getSourceValue(sources[0])) {
            is Tracer.TracedValue.Constant -> constants[traced.constant]
            is Tracer.TracedValue.Data,
            is Tracer.TracedValue.Phi -> return this
        }

        val value = when (valueA) {
            is Value.Integer -> {
                when (instruction.opcode) {
                    Opcode.NEGATIVE -> Value.Integer(-valueA.value)
                    else -> return this
                }
            }

            is Value.Decimal -> {
                when (instruction.opcode) {
                    Opcode.NEGATIVE -> Value.Decimal(-valueA.value)
                    else -> return this
                }
            }

            is Value.Boolean -> {
                when (instruction.opcode) {
                    Opcode.NOT -> Value.Boolean(!valueA.value)
                    Opcode.NEGATIVE -> Value.Boolean(!valueA.value)
                    else -> return this
                }
            }

            is Value.String -> {
                when (instruction.opcode) {
                    Opcode.LENGTH -> Value.Integer(valueA.value.length)
                    else -> return this
                }
            }

            else -> return this
        }

        return value.toInstruction(constants, destination, instruction.lineData)
    }

    private fun AbstractInstruction.Processing.equal(
        wrapper: Tracer.Node,
        constants: MutableList<Value>,
    ): AbstractInstruction {
        val valueA = when (val traced = wrapper.getSourceValue(sources[0])) {
            is Tracer.TracedValue.Constant -> constants[traced.constant]
            is Tracer.TracedValue.Data,
            is Tracer.TracedValue.Phi -> return this
        }

        val valueB = when (val traced = wrapper.getSourceValue(sources[1])) {
            is Tracer.TracedValue.Constant -> constants[traced.constant]
            is Tracer.TracedValue.Data,
            is Tracer.TracedValue.Phi -> return this
        }

        val value = when {
            valueA is Value.Integer && valueB is Value.Integer -> Value.Boolean(valueA.value == valueB.value)
            valueA is Value.Decimal && valueB is Value.Decimal -> Value.Boolean(valueA.value == valueB.value)
            valueA is Value.Boolean && valueB is Value.Boolean -> Value.Boolean(valueA.value == valueB.value)
            valueA is Value.String && valueB is Value.String -> Value.Boolean(valueA.value == valueB.value)

            else -> return this
        }

        return value.toInstruction(constants, destination, instruction.lineData)
    }

    private fun AbstractInstruction.Processing.binary(
        wrapper: Tracer.Node,
        constants: MutableList<Value>,
    ): AbstractInstruction {
        val valueA = when (val traced = wrapper.getSourceValue(sources[0])) {
            is Tracer.TracedValue.Constant -> constants[traced.constant]
            is Tracer.TracedValue.Data,
            is Tracer.TracedValue.Phi -> return this
        }

        val valueB = when (val traced = wrapper.getSourceValue(sources[1])) {
            is Tracer.TracedValue.Constant -> constants[traced.constant]
            is Tracer.TracedValue.Data,
            is Tracer.TracedValue.Phi -> return this
        }

        val value = when {
            valueA is Value.Integer && valueB is Value.Integer -> {
                when (instruction.opcode) {
                    Opcode.ADD -> Value.Integer(valueA.value + valueB.value)
                    Opcode.SUB -> Value.Integer(valueA.value - valueB.value)
                    Opcode.MUL -> Value.Integer(valueA.value * valueB.value)
                    Opcode.DIV -> Value.Integer(valueA.value / valueB.value)
                    Opcode.MOD -> Value.Integer(valueA.value % valueB.value)
                    Opcode.LESS -> Value.Boolean(valueA.value < valueB.value)
                    Opcode.LESS_EQUAL -> Value.Boolean(valueA.value <= valueB.value)
                    else -> return this
                }
            }

            valueA is Value.Decimal && valueB is Value.Decimal -> {
                when (instruction.opcode) {
                    Opcode.ADD -> Value.Decimal(valueA.value + valueB.value)
                    Opcode.SUB -> Value.Decimal(valueA.value - valueB.value)
                    Opcode.MUL -> Value.Decimal(valueA.value * valueB.value)
                    Opcode.DIV -> Value.Decimal(valueA.value / valueB.value)
                    Opcode.MOD -> Value.Decimal(valueA.value % valueB.value)
                    Opcode.LESS -> Value.Boolean(valueA.value < valueB.value)
                    Opcode.LESS_EQUAL -> Value.Boolean(valueA.value <= valueB.value)
                    else -> return this
                }
            }

            valueA is Value.Boolean && valueB is Value.Boolean -> {
                when (instruction.opcode) {
                    Opcode.AND -> Value.Boolean(valueA.value && valueB.value)
                    Opcode.OR -> Value.Boolean(valueA.value || valueB.value)
                    else -> return this
                }
            }

            valueA is Value.String && valueB is Value.String -> {
                when (instruction.opcode) {
                    Opcode.CONCAT -> Value.String(valueA.value + valueB.value)
                    else -> return this
                }
            }

            else -> return this
        }

        return value.toInstruction(constants, destination, instruction.lineData)
    }

    private fun Value.toInstruction(
        constants: MutableList<Value>,
        destination: Argument.Slot,
        lineData: LineData?
    ): AbstractInstruction {
        constants.add(this)

        val instruction = Instruction.Load(
            constant = Argument.Constant(constants.size - 1),
            destination = destination,
            lineData = lineData
        )

        return instruction.abstract()
    }
}