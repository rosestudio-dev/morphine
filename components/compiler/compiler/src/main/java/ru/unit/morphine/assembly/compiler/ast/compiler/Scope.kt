package ru.unit.morphine.assembly.compiler.ast.compiler

class Scope<T : Scope.Slot>(private val slots: MutableList<Slot>) {

    private val levelStack = mutableListOf(Level())

    val levels
        get() = levelStack.map { level ->
            level.indices.map { index ->
                IndexedValue(
                    index = index,
                    value = slots[index] as T
                )
            }
        }

    fun enter() {
        levelStack.add(Level())
    }

    fun exit() {
        val level = levelStack.removeLast()

        level.indices.forEach { index ->
            slots[index] = Empty
        }
    }

    fun add(slot: T): Int {
        val emptyIndex = slots.indexOfFirst { s -> s is Empty }

        val index = if (emptyIndex in slots.indices) {
            slots[emptyIndex] = slot
            emptyIndex
        } else {
            slots.add(slot)
            slots.size - 1
        }

        levelStack.last().indices.add(index)

        return index
    }

    sealed interface Slot {
        data object Temporary : Slot
        data class Variable(
            val name: String,
            val isConst: Boolean
        ) : Slot

    }

    private data class Level(
        val indices: MutableList<Int> = mutableListOf()
    )

    private data object Empty : Slot
}