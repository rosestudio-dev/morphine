package ru.unit.morphine.assembly.optimizer.tracer

import org.jgrapht.graph.DefaultDirectedGraph
import org.jgrapht.graph.DefaultEdge
import org.jgrapht.traverse.DepthFirstIterator

class ControlFlowTree(
    val originalData: Tracer.Data
) {
    val graph = DefaultDirectedGraph<Tracer.Block, DefaultEdge>(DefaultEdge::class.java).apply {
        originalData.blocks.forEach { block ->
            addVertex(block)

            block.edges(originalData).forEach { end ->
                if (!containsVertex(end)) {
                    addVertex(end)
                }

                addEdge(block, end)
            }
        }

        val iter = DepthFirstIterator(this, originalData.blocks.first()).apply {
            isCrossComponentTraversal = false
        }

        val travel = mutableListOf<Tracer.Block>()

        while (iter.hasNext()) {
            travel.add(iter.next())
        }

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
