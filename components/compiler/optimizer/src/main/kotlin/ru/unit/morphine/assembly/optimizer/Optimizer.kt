package ru.unit.morphine.assembly.optimizer

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.optimizer.strategy.CalculationStrategy
import ru.unit.morphine.assembly.optimizer.strategy.ConstantZipperStrategy
import ru.unit.morphine.assembly.optimizer.strategy.DestinationPropagationStrategy
import ru.unit.morphine.assembly.optimizer.strategy.LinearBranchStrategy
import ru.unit.morphine.assembly.optimizer.strategy.MeaninglessStrategy
import ru.unit.morphine.assembly.optimizer.strategy.PositionPropagationStrategy
import ru.unit.morphine.assembly.optimizer.strategy.SlotZipperStrategy
import ru.unit.morphine.assembly.optimizer.strategy.SourcePropagationStrategy
import ru.unit.morphine.assembly.optimizer.strategy.StubStrategy
import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import ru.unit.morphine.assembly.optimizer.tracer.functions.tracerPrint

class Optimizer(private val bytecode: Bytecode) {

    private companion object {
        const val MAX_ITERATIONS = 262144
    }

    private val tracer = Tracer()

    private val repeatableStrategies = listOf(
        DestinationPropagationStrategy(),
        SourcePropagationStrategy(),
        CalculationStrategy(),
        LinearBranchStrategy(),
        PositionPropagationStrategy(),
        MeaninglessStrategy(),
        StubStrategy(),
    )

    private val onceStrategies = listOf(
        SlotZipperStrategy(),
        ConstantZipperStrategy()
    )

    fun optimize(): Bytecode {
        val optimized = bytecode.functions.map { function ->
            if (function.optimize) {
                function.optimize()
            } else {
                function
            }
        }

        return bytecode.copy(
            functions = optimized
        )
    }

    private fun Bytecode.Function.optimize() =
        repeatableOptimize().optimizeWithStrategies(onceStrategies)

    private fun Bytecode.Function.repeatableOptimize() =
        (0 until MAX_ITERATIONS).fold(this) { last, _ ->
            val result = last.optimizeWithStrategies(repeatableStrategies)

            if (result == last) {
                return result
            } else {
                return@fold result
            }
        }

    private fun Bytecode.Function.optimizeWithStrategies(
        strategies: List<OptimizationStrategy>
    ) = strategies.fold(this) { current, strategy ->
        val data = tracer.trace(current)
        val out = strategy.optimize(data)
        tracer.reconstruct(out)
    }
}
