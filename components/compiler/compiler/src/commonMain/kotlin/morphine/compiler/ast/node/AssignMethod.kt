package morphine.compiler.ast.node

sealed interface AssignMethod<T> {

    data class Decompose<T>(
        val entries: List<Entry<T>>,
    ) : AssignMethod<T> {

        data class Entry<T>(
            val value: T,
            val key: Expression
        )
    }

    data class Single<T>(
        val entry: T
    ) : AssignMethod<T>
}