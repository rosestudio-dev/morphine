package ru.why.morphine.jni

sealed class MorphineException(message: String) : RuntimeException(message) {
    class Panic(message: String) : MorphineException(message)
    class Error(message: String) : MorphineException(message)
    class Other(message: String) : MorphineException(message)
}
