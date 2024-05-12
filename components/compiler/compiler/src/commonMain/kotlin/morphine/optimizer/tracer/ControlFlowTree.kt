package morphine.optimizer.tracer

import morphine.utils.Graph

class ControlFlowTree(
    val originalData: Tracer.Data
) {
    val graph = Graph<Tracer.Block>().apply {
        originalData.blocks.forEach { block ->
            addVertex(block)

            block.edges(originalData).forEach { end ->
                if (!containsVertex(end)) {
                    addVertex(end)
                }

                addEdge(block, end)
            }
        }

        val travel = dfi(originalData.blocks.first()).asSequence().toList()

        originalData.blocks.filter { block ->
            block !in travel
        }.forEach(this::removeVertex)
    }

    private val blocks = graph.vertexSet().sortedBy(Tracer.Block::start)

    private val nodes = blocks.flatMap { block ->
        originalData.nodes.subList(block.start, block.end)
    }

    val data = originalData.copy(
        nodes = nodes,
        blocks = blocks
    )

    val dominatorTree = DominatorTree(graph, blocks.first())
}
