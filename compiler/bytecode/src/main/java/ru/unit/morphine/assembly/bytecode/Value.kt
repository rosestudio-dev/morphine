package ru.unit.morphine.assembly.bytecode

import java.util.UUID

sealed interface Value {

    data object Nil : Value

    data class Integer(
        val value: Int
    ) : Value

    data class Decimal(
        val value: Double
    ) : Value

    data class Boolean(
        val value: kotlin.Boolean
    ) : Value

    data class String(
        val value: kotlin.String
    ) : Value

    data class Function(
        val value: UUID
    ) : Value
}