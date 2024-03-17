package ru.unit.morphine.assembly.bytecode.processor.provider

import com.google.devtools.ksp.processing.SymbolProcessorEnvironment
import com.google.devtools.ksp.processing.SymbolProcessorProvider
import ru.unit.morphine.assembly.bytecode.processor.AnnotationProcessor

class AnnotationProcessorProvider : SymbolProcessorProvider {

    override fun create(environment: SymbolProcessorEnvironment) =
        AnnotationProcessor(environment)
}