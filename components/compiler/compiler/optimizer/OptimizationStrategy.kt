package morphine.optimizer

import morphine.optimizer.tracer.Tracer

interface OptimizationStrategy {

    fun optimize(data: Tracer.Data): Tracer.Data
}