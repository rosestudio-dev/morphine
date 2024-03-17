package ru.unit.orchestra_features.processor.utils.extension

import com.google.devtools.ksp.symbol.KSAnnotated
import com.squareup.kotlinpoet.ksp.toClassName

inline fun <reified T> KSAnnotated.getAnnotations() = annotations.filter { annotation ->
    val annotationClass = annotation.annotationType.resolve().toClassName().canonicalName
    val genericClass = T::class.qualifiedName

    annotationClass == genericClass
}

inline fun <reified T> KSAnnotated.getAnnotation() = getAnnotations<T>().lastOrNull()