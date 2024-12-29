package ru.why.morphine.jni

sealed interface MorphineValue {
    sealed interface Primitive : MorphineValue

    data object Nil : Primitive
    data class Integer(@JvmField val value: Long) : Primitive
    data class Decimal(@JvmField val value: Double) : Primitive
    data class Boolean(@JvmField val value: kotlin.Boolean) : Primitive
    data class String(@JvmField val value: kotlin.String) : Primitive

    class Table(@JvmField val value: Map<Primitive, Primitive>) : MorphineValue {
        override fun toString() = value.toString()
    }

    class Vector(@JvmField val value: List<Primitive>) : MorphineValue {
        override fun toString() = value.toString()
    }
}

private fun Any?.toMorphinePrimitiveValue() = when (this) {
    null -> MorphineValue.Nil
    is Int -> MorphineValue.Integer(this.toLong())
    is Long -> MorphineValue.Integer(this)
    is Float -> MorphineValue.Decimal(this.toDouble())
    is Double -> MorphineValue.Decimal(this)
    is Boolean -> MorphineValue.Boolean(this)
    is String -> MorphineValue.String(this)
    else -> throw Exception("Cannot convert ${this.javaClass.canonicalName} to morphine value")
}

fun Any?.toMorphineValue(): MorphineValue = when (this) {
    is Map<*, *> -> {
        val map = this.entries.associate { entry ->
            entry.key.toMorphinePrimitiveValue() to entry.value.toMorphinePrimitiveValue()
        }
        MorphineValue.Table(map)
    }

    is List<*> -> {
        val list = this.map { value -> value.toMorphinePrimitiveValue() }
        MorphineValue.Vector(list)
    }

    is Array<*> -> {
        val list = this.map { value -> value.toMorphinePrimitiveValue() }
        MorphineValue.Vector(list)
    }

    else -> this.toMorphinePrimitiveValue()
}
