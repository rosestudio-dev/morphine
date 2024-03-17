package ru.unit.orchestra_features.processor.utils.extension

import java.util.Locale

fun String.capitalize(): String = replaceFirstChar {
    if (it.isLowerCase()) {
        it.titlecase(Locale.getDefault())
    } else {
        it.toString()
    }
}