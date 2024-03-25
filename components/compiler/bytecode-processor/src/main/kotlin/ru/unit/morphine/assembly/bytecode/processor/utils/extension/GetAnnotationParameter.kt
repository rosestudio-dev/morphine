package ru.unit.orchestra_features.processor.utils.extension

import com.google.devtools.ksp.symbol.KSAnnotation
import com.squareup.kotlinpoet.ksp.toClassName

fun KSAnnotation?.getRawParameter(name: String): Any? {
    this ?: return null

    val parameter = arguments.firstOrNull { argument ->
        argument.name?.asString() == name
    } ?: throw Exception("Parameter of ${annotationType.resolve().toClassName().simpleName} wasn't found")

    return parameter.value
}

inline fun <reified T> KSAnnotation?.getParameter(name: String, default: T) =
    getRawParameter(name)?.let { value ->
        if (value is T) {
            value
        } else {
            null
        }
    } ?: default
