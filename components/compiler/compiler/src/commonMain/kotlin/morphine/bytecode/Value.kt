package morphine.bytecode

import morphine.utils.UID

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
        val value: UID
    ) : Value
}