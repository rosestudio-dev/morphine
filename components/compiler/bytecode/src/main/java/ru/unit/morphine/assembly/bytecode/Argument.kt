package ru.unit.morphine.assembly.bytecode

sealed interface Argument {

    data class Constant(
        override val value: Int
    ) : Argument

    data class Slot(
        override val value: Int,
    ) : Argument

    data class Arg(
        override val value: Int,
    ) : Argument

    data class Param(
        override val value: Int,
    ) : Argument

    data class Index(
        override val value: Int
    ) : Argument

    data class Count(
        override val value: Int
    ) : Argument

    data class Position(
        override val value: Int
    ) : Argument

    val value: Int
}