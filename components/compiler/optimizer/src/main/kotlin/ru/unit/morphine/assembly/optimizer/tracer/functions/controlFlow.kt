package ru.unit.morphine.assembly.optimizer.tracer.functions

import ru.unit.morphine.assembly.optimizer.tracer.ControlFlowTree
import ru.unit.morphine.assembly.optimizer.tracer.Tracer

fun Tracer.Data.controlFlow() =
    ControlFlowTree(this).also { tree ->
        controlFlowTree = tree
    }
