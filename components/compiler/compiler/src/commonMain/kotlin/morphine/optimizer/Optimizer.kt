package morphine.optimizer

import morphine.bytecode.Bytecode
import morphine.optimizer.strategy.CalculationStrategy
import morphine.optimizer.strategy.ConstantZipperStrategy
import morphine.optimizer.strategy.DestinationPropagationStrategy
import morphine.optimizer.strategy.LinearBranchStrategy
import morphine.optimizer.strategy.MeaninglessStrategy
import morphine.optimizer.strategy.PositionPropagationStrategy
import morphine.optimizer.strategy.SlotZipperStrategy
import morphine.optimizer.strategy.SourcePropagationStrategy
import morphine.optimizer.strategy.StubStrategy
import morphine.optimizer.tracer.Tracer
import morphine.optimizer.tracer.functions.tracerPrint

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
