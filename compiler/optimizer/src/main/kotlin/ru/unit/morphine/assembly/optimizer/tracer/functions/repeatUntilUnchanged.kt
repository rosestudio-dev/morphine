package ru.unit.morphine.assembly.optimizer.tracer.functions

fun <T> repeatUntilUnchanged(
    init: () -> List<T>,
    copy: T.() -> T,
    progress: List<T>.() -> List<T>
): List<T> {
    var data = init()

    while (true) {
        val old = data.map(copy)

        val new = progress(data)

        if (old == new) {
            return new
        } else {
            data = new
        }
    }
}