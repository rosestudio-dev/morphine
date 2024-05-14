package morphine.optimizer.tracer

import morphine.optimizer.exception.OptimizerException
import morphine.utils.Graph

class DominatorTree<T>(
    private val graph: Graph<T>,
    val root: T
) {
    val iDomGraph: Graph<T>

    private val nodePreOrder = root?.let { first ->
        dfs(graph, first)
    } ?: emptyList()

    private val idomMap = mutableMapOf<T, T>()
    private val frontMap = mutableMapOf<T, MutableSet<T>>()
    private val preOrderMap = mutableMapOf<T, Int>()

    init {
        iDomGraph = create()
        calcFronts()
    }

    fun isDominates(dominator: T, dominated: T): Boolean {
        if (dominator == dominated) {
            return true
        }

        var dom = idomMap[dominated]

        while ((dom != null) && (preOrderMap[dom]!! >= preOrderMap[dominator]!!) && (dom != dominator)) {
            dom = idomMap[dom]!!
        }

        return dominator == dom
    }

    fun getIDom(t: T): T {
        return idomMap[t]!!
    }

    fun getFronts(t: T): Set<T> {
        return frontMap[t]!!.toSet()
    }

    private fun calcFronts() {
        idomMap.forEach { (dominated, _) ->
            frontMap[dominated] = mutableSetOf()
        }

        idomMap.forEach { (dominated, _) ->
            val strictlyVertexes = iDomGraph.outgoingEdges(dominated).flatMap { edge ->
                dfs(iDomGraph, edge.target)
            }

            val fronts = graph.vertexSet().filterNot { vertex ->
                vertex in strictlyVertexes
            }.filter { vertex ->
                val ingoingVertexes = graph.incomingEdges(vertex).map { edge ->
                    edge.source
                }

                ingoingVertexes.any { ingoingVertex ->
                    isDominates(dominated, ingoingVertex)
                }
            }

            frontMap[dominated]!!.addAll(fronts)
        }
    }

    private fun create(): Graph<T> {
        nodePreOrder.forEachIndexed { index, node ->
            preOrderMap[node] = index
        }

        computeDominators()

        return getDominatorTree()
    }

    private fun dfs(graph: Graph<T>, startNode: T): List<T> {
        val iter = graph.dfi(startNode)
        val travel = mutableListOf<T>()

        while (iter.hasNext()) {
            travel.add(iter.next())
        }

        return travel
    }

    private fun computeDominators() {
        val firstElement = nodePreOrder.firstOrNull() ?: return
        idomMap[firstElement] = firstElement

        var changed: Boolean

        do {
            changed = false

            for (node in nodePreOrder) {
                if (node == firstElement) {
                    continue
                }

                val oldIdom = idomMap[node]
                var newIdom: T? = null

                for (edge in graph.incomingEdges(node)) {
                    val preNode = edge.source

                    if (idomMap[preNode] == null) {
                        continue
                    }

                    if (newIdom == null) {
                        newIdom = preNode
                    } else {
                        newIdom = intersectIDoms(preNode, newIdom)
                    }
                }

                if (newIdom == null) {
                    throw OptimizerException("null idom ($node)")
                }

                if (newIdom != oldIdom) {
                    changed = true

                    idomMap[node] = newIdom
                }
            }
        } while (changed)
    }

    private fun intersectIDoms(a: T, b: T): T? {
        var t1: T? = a
        var t2: T? = b

        while (t1 != t2) {
            if (preOrderMap[t1]!! < preOrderMap[t2]!!) {
                t2 = idomMap[t2]
            } else {
                t1 = idomMap[t1]
            }
        }

        return t1
    }

    private fun getDominatorTree(): Graph<T> {
        val tree = Graph<T>()
        for (node in graph.vertexSet()) {
            tree.addVertex(node)

            val idom = idomMap[node] ?: continue

            if (node != idom) {
                tree.addVertex(idom)
                tree.addEdge(idom, node)
            }
        }

        return tree
    }
}
