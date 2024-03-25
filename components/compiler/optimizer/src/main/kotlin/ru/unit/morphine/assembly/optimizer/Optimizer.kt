package ru.unit.morphine.assembly.optimizer

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.optimizer.strategy.*
import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import ru.unit.morphine.assembly.optimizer.tracer.functions.tracerPrint

class Optimizer(
    private val debug: Boolean
) {

    private companion object {
        const val MAX_ITERATIONS = 262144
    }

    private val tracer = Tracer(debug)

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

    fun optimize(bytecode: Bytecode): Bytecode {
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
        repeatableOptimize().optimizeWithStrategies(onceStrategies, -1)

    private fun Bytecode.Function.repeatableOptimize() =
        (0 until MAX_ITERATIONS).fold(this) { last, iteration ->
            val result = last.optimizeWithStrategies(repeatableStrategies, iteration)

            if (result == last) {
                return result
            } else {
                return@fold result
            }
        }

    private fun Bytecode.Function.optimizeWithStrategies(
        strategies: List<OptimizationStrategy>,
        iteration: Int
    ) = strategies.fold(this) { current, strategy ->
        val data = tracer.trace(current)
        val out = strategy.optimize(data)

        if (debug) {
            optimizePrint(data, out, strategy, iteration)
        }

        tracer.reconstruct(out)
    }

    private fun optimizePrint(data: Tracer.Data, out: Tracer.Data, strategy: OptimizationStrategy, iteration: Int) {
        println("Strategy: ${strategy::class.simpleName}, iteration: $iteration")
        data.tracerPrint(out)
        println()
    }
}
