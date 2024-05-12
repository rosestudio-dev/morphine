package morphine.processor.bytecode

import com.google.devtools.ksp.symbol.KSFile
import com.squareup.kotlinpoet.ClassName
import kotlin.reflect.KClass

data class Model(
    val type: Type<*>,
    val instruction: ClassName,
    val destination: String?,
    val sources: List<String>,
    val file: KSFile
) {

    sealed interface Type<T : Annotation> {

        val kclass: KClass<T>

        data object Consumer : Type<morphine.annotation.bytecode.Consumer> {

            override val kclass: KClass<morphine.annotation.bytecode.Consumer> =
                morphine.annotation.bytecode.Consumer::class
        }

        data object Control : Type<morphine.annotation.bytecode.Control> {

            override val kclass: KClass<morphine.annotation.bytecode.Control> =
                morphine.annotation.bytecode.Control::class
        }

        data object Movement : Type<morphine.annotation.bytecode.Movement> {

            override val kclass: KClass<morphine.annotation.bytecode.Movement> =
                morphine.annotation.bytecode.Movement::class
        }

        data object Processing : Type<morphine.annotation.bytecode.Processing> {

            override val kclass: KClass<morphine.annotation.bytecode.Processing> =
                morphine.annotation.bytecode.Processing::class
        }

        data object Producer : Type<morphine.annotation.bytecode.Producer> {
            override val kclass: KClass<morphine.annotation.bytecode.Producer> =
                morphine.annotation.bytecode.Producer::class
        }
    }
}