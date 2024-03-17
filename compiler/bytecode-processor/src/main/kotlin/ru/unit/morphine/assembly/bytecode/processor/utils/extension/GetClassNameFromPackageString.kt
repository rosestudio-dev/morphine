package ru.unit.orchestra_features.processor.utils.extension

import com.squareup.kotlinpoet.ClassName

fun String.asClassName(isNullable: Boolean = false): ClassName {
    val splitClassName = split(".")
    val packageName = splitClassName.dropLast(1).joinToString(".")
    val className = splitClassName.last()

    return ClassName(packageName, className)
}