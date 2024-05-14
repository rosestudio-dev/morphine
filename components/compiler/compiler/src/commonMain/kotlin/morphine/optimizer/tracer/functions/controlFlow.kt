package morphine.optimizer.tracer.functions

import morphine.optimizer.tracer.ControlFlowTree
import morphine.optimizer.tracer.Tracer

fun Tracer.Data.controlFlow() =
    ControlFlowTree(this).also { tree ->
        controlFlowTree = tree
    }
