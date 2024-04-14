package ru.unit.morphine.assembly.bytecode.processor

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

        data object Consumer : Type<ru.unit.morphine.assembly.bytecode.annotation.Consumer> {

            override val kclass: KClass<ru.unit.morphine.assembly.bytecode.annotation.Consumer> =
                ru.unit.morphine.assembly.bytecode.annotation.Consumer::class
        }

        data object Control : Type<ru.unit.morphine.assembly.bytecode.annotation.Control> {

            override val kclass: KClass<ru.unit.morphine.assembly.bytecode.annotation.Control> =
                ru.unit.morphine.assembly.bytecode.annotation.Control::class
        }

        data object Movement : Type<ru.unit.morphine.assembly.bytecode.annotation.Movement> {

            override val kclass: KClass<ru.unit.morphine.assembly.bytecode.annotation.Movement> =
                ru.unit.morphine.assembly.bytecode.annotation.Movement::class
        }

        data object Processing : Type<ru.unit.morphine.assembly.bytecode.annotation.Processing> {

            override val kclass: KClass<ru.unit.morphine.assembly.bytecode.annotation.Processing> =
                ru.unit.morphine.assembly.bytecode.annotation.Processing::class
        }

        data object Producer : Type<ru.unit.morphine.assembly.bytecode.annotation.Producer> {
            override val kclass: KClass<ru.unit.morphine.assembly.bytecode.annotation.Producer> =
                ru.unit.morphine.assembly.bytecode.annotation.Producer::class
        }
    }
}