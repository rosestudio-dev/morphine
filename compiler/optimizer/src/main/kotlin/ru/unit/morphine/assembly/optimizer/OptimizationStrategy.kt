package ru.unit.morphine.assembly.optimizer

import ru.unit.morphine.assembly.optimizer.tracer.Tracer

interface OptimizationStrategy {

    fun optimize(data: Tracer.Data): Tracer.Data
}