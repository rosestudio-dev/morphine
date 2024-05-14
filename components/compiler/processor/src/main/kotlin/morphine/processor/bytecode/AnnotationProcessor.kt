package morphine.processor.bytecode

import com.google.devtools.ksp.processing.Dependencies
import com.google.devtools.ksp.processing.Resolver
import com.google.devtools.ksp.processing.SymbolProcessor
import com.google.devtools.ksp.processing.SymbolProcessorEnvironment
import com.google.devtools.ksp.symbol.KSAnnotated
import com.google.devtools.ksp.validate
import com.squareup.kotlinpoet.ksp.writeTo
import kotlin.reflect.KClass
import morphine.annotation.bytecode.Consumer
import morphine.annotation.bytecode.Control
import morphine.annotation.bytecode.Movement
import morphine.annotation.bytecode.Processing
import morphine.annotation.bytecode.Producer

class AnnotationProcessor(
    private val environment: SymbolProcessorEnvironment,
) : SymbolProcessor {

    override fun process(resolver: Resolver): List<KSAnnotated> {
        val consumeAnnotations = resolver.findAnnotations(Consumer::class).toList()
        val controlAnnotations = resolver.findAnnotations(Control::class).toList()
        val movementAnnotations = resolver.findAnnotations(Movement::class).toList()
        val processingAnnotations = resolver.findAnnotations(Processing::class).toList()
        val producerAnnotations = resolver.findAnnotations(Producer::class).toList()

        val notValidated = (consumeAnnotations +
                controlAnnotations +
                movementAnnotations +
                processingAnnotations +
                producerAnnotations).filterNot(KSAnnotated::validate)

        val models = consumeAnnotations.map { node ->
            node.accept(Visitor(), Model.Type.Consumer)
        } + controlAnnotations.map { node ->
            node.accept(Visitor(), Model.Type.Control)
        } + movementAnnotations.map { node ->
            node.accept(Visitor(), Model.Type.Movement)
        } + processingAnnotations.map { node ->
            node.accept(Visitor(), Model.Type.Processing)
        } + producerAnnotations.map { node ->
            node.accept(Visitor(), Model.Type.Producer)
        }

        val packagePrefix = environment.options[PACKAGE_PREFIX_ARG]?.plus(".") ?: ""

        if (models.isNotEmpty()) {
            val file = Code(
                environment = environment,
                packageData = PackageData(
                    name = "$packagePrefix$PACKAGE",
                    suffix = PACKAGE
                )
            ).generate(models)

            file.writeTo(
                codeGenerator = environment.codeGenerator,
//                dependencies = Dependencies.ALL_FILES
                Dependencies(
                    aggregating = true,
                    *models.map(Model::file).toTypedArray()
                )
            )
        }

        return notValidated
    }

    private fun Resolver.findAnnotations(
        kClass: KClass<*>,
    ) = getSymbolsWithAnnotation(kClass.qualifiedName.toString())

    companion object {

        const val PACKAGE_PREFIX_ARG = "processor.packagePrefix"
        const val PACKAGE = "generated"
    }
}