package morphine.processor.bytecode

import kotlin.reflect.KClass

sealed interface ModelType<T : Annotation> {

    val kclass: KClass<T>

    data object Consumer : ModelType<morphine.annotation.bytecode.Consumer> {

        override val kclass: KClass<morphine.annotation.bytecode.Consumer> =
            morphine.annotation.bytecode.Consumer::class
    }

    data object Clean : ModelType<morphine.annotation.bytecode.Clean> {

        override val kclass: KClass<morphine.annotation.bytecode.Clean> =
            morphine.annotation.bytecode.Clean::class
    }

    data object Control : ModelType<morphine.annotation.bytecode.Control> {

        override val kclass: KClass<morphine.annotation.bytecode.Control> =
            morphine.annotation.bytecode.Control::class
    }

    data object Movement : ModelType<morphine.annotation.bytecode.Movement> {

        override val kclass: KClass<morphine.annotation.bytecode.Movement> =
            morphine.annotation.bytecode.Movement::class
    }

    data object Processing : ModelType<morphine.annotation.bytecode.Processing> {

        override val kclass: KClass<morphine.annotation.bytecode.Processing> =
            morphine.annotation.bytecode.Processing::class
    }

    data object Producer : ModelType<morphine.annotation.bytecode.Producer> {
        override val kclass: KClass<morphine.annotation.bytecode.Producer> =
            morphine.annotation.bytecode.Producer::class
    }
}