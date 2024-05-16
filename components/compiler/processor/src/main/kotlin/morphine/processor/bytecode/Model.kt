package morphine.processor.bytecode

import com.google.devtools.ksp.symbol.KSFile
import com.squareup.kotlinpoet.ClassName

sealed interface Model {

    val file: KSFile
    val instruction: ClassName

    data class Clean(
        val from: String,
        val count: String,
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model

    data class Consumer(
        val sources: List<String>,
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model {

        init {
            assert(sources.isNotEmpty())
        }
    }

    data class Control(
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model

    data class Movement(
        val source: String,
        val destination: String,
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model

    data class Processing(
        val sources: List<String>,
        val destination: String,
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model {

        init {
            assert(sources.isNotEmpty())
        }
    }

    data class Producer(
        val destination: String,
        override val file: KSFile,
        override val instruction: ClassName,
    ) : Model
}