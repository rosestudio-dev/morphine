package ru.unit.morphine.assembly.optimizer.tracer

class Graph<T> {

    private val vertexes = mutableSetOf<T>()
    private val edges = mutableSetOf<Edge<T>>()

    fun addVertex(vertex: T) {
        vertexes.add(vertex)
    }

    fun addEdge(source: T, target: T) {
        if (source !in vertexes || target !in vertexes) {
            throw RuntimeException("Vertex not found")
        }

        val edge = Edge(
            source = source,
            target = target
        )

        edges.add(edge)
    }

    fun removeVertex(vertex: T): Boolean {
        val found = edges.any { (source, target) ->
            source == vertex || target == vertex
        }

        if (found) {
            throw RuntimeException("Vertex used in edge")
        }

        return vertexes.remove(vertex)
    }

    fun removeEdge(edge: Edge<T>) = edges.remove(edge)

    fun containsVertex(vertex: T) = vertex in vertexes

    fun vertexSet() = vertexes.toSet()

    fun incomingEdges(vertex: T) = edges.filter { edge -> edge.target == vertex }

    fun outgoingEdges(vertex: T) = edges.filter { edge -> edge.source == vertex }

    fun dfi(headVertex: T) = DepthFirstIterator(headVertex)

    fun bfi() = BreadthFirstIterator()

    inner class DepthFirstIterator(headVertex: T) : Iterator<T> {

        private val visited = mutableSetOf<T>()
        private val stack = mutableListOf(headVertex)

        override fun hasNext() = stack.isNotEmpty()

        override fun next(): T {
            val vertex = stack.removeLast()

            visited.add(vertex)

            val edges = outgoingEdges(vertex)
                .map(Edge<T>::target)
                .filter { target -> target !in visited }

            stack.addAll(edges)

            return vertex
        }
    }

    inner class BreadthFirstIterator : Iterator<T> {

        private val iterator = vertexes.iterator()

        override fun hasNext() = iterator.hasNext()

        override fun next() = iterator.next()
    }

    data class Edge<T>(
        val source: T,
        val target: T
    )
}