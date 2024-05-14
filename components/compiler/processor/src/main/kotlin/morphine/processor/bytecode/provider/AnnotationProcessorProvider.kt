package morphine.processor.bytecode.provider

import com.google.devtools.ksp.processing.SymbolProcessorEnvironment
import com.google.devtools.ksp.processing.SymbolProcessorProvider
import morphine.processor.bytecode.AnnotationProcessor

class AnnotationProcessorProvider : SymbolProcessorProvider {

    override fun create(environment: SymbolProcessorEnvironment) =
        AnnotationProcessor(environment)
}