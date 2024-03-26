package ru.unit.morphine.assembly.optimizer.tracer.functions

import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import kotlin.math.max

fun Tracer.Data.tracerPrint(
    out: Tracer.Data? = null,
    usedMask: Boolean = true,
    blockNewLine: Boolean = true
): Boolean {
    var has = false
    val result = buildString {
        append("Instructions:\n")

        fun <T> List<T>.joinToStringUsable(
            use: List<Tracer.TracedUse>,
            transform: ((T) -> CharSequence)? = null
        ): String {
            return mapIndexed { index, t ->
                when {
                    !usedMask -> t.toString()

                    index !in use.indices -> t.toString()

                    use[index].used -> if (transform == null) {
                        t.toString()
                    } else {
                        transform(t)
                    }

                    else -> "#"
                }
            }.joinToString()
        }

        fun appendBeforeAfter(
            before: String,
            after: String,
            sizeBefore: Int,
            sizeAfter: Int
        ) {
            append(" | ")

            append(
                after.let { str ->
                    str + " ".repeat(sizeAfter - str.length)
                }
            )

            append(" <- ")

            append(
                before.let { str ->
                    str + " ".repeat(sizeBefore - str.length)
                }
            )
        }

        val sizeBlock = blocks.maxOfOrNull { block ->
            val edges = block.edges(this@tracerPrint)

            buildString {
                append("block ${block.id}")
                if (edges.isNotEmpty()) {
                    append(" (edges ${edges.joinToString { it.id.toString() }})")
                }
                append(": ")
            }.length
        } ?: 4

        val sizeInstruction = nodes.maxOfOrNull { instruction ->
            val name = instruction.abstract.instruction.opcode.name.lowercase()
            val args = instruction.abstract.instruction.orderedArguments.joinToString()

            "# $name $args".length
        } ?: 0

        val sizeIndex = "${max(nodes.size - 1, 0)}. ".length

        val sizeDestinations = nodes.maxOfOrNull { wrapper ->
            wrapper.destinations.joinToString().length
        } ?: 0

        val sizeVersionsBefore = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedVersionsBefore.joinToStringUsable(wrapper.tracedUsesBefore)}]".length
        } ?: 0

        val sizeVersionsAfter = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedVersionsAfter.joinToStringUsable(wrapper.tracedUsesAfter)}]".length
        } ?: 0


        val sizeValuesBefore = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedValuesBefore.joinToStringUsable(wrapper.tracedUsesBefore)}]".length
        } ?: 0

        val sizeValuesAfter = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedValuesAfter.joinToStringUsable(wrapper.tracedUsesAfter)}]".length
        } ?: 0


        val sizeUsesBefore = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedUsesBefore.joinToString()}]".length
        } ?: 0

        val sizeUsesAfter = nodes.maxOfOrNull { wrapper ->
            "[${wrapper.tracedUsesAfter.joinToString()}]".length
        } ?: 0

        nodes.forEachIndexed { index, wrapper ->
            val block = blocks.find { block -> block.start == index }

            val blockStr = if (block != null) {
                val edges = block.edges(this@tracerPrint)
                val str = buildString {
                    append("block ${block.id}")
                    if (edges.isNotEmpty()) {
                        append(" (edges ${edges.joinToString { it.id.toString() }})")
                    }
                    append(": ")
                }

                " ".repeat(sizeBlock - str.length) + str
            } else {
                " ".repeat(sizeBlock)
            }

            val name = wrapper.abstract.instruction.opcode.name.lowercase()
            val args = wrapper.abstract.instruction.orderedArguments.joinToString()
            val destinations = wrapper.destinations.joinToString()

            val available = when {
                out == null -> " "

                !out.nodes[index].isAvailable -> {
                    has = true
                    "-"
                }

                wrapper.abstract != out.nodes[index].abstract -> {
                    has = true
                    "*"
                }

                else -> " "
            }

            val versionsBefore = "[${wrapper.tracedVersionsBefore.joinToStringUsable(wrapper.tracedUsesBefore)}]"
            val versionsAfter = "[${wrapper.tracedVersionsAfter.joinToStringUsable(wrapper.tracedUsesAfter)}]"

            val valuesBefore = "[${wrapper.tracedValuesBefore.joinToStringUsable(wrapper.tracedUsesBefore)}]"
            val valuesAfter = "[${wrapper.tracedValuesAfter.joinToStringUsable(wrapper.tracedUsesAfter)}]"

            val usesBefore = "[${wrapper.tracedUsesBefore.joinToString()}]"
            val usesAfter = "[${wrapper.tracedUsesAfter.joinToString()}]"

            append(
                if (blockNewLine) {
                    blockStr.trim()
                } else {
                    blockStr
                }
            )

            when {
                blockNewLine && block != null -> append("\n    ")
                blockNewLine && block == null -> append("    ")
            }

            append(
                "${index}. ".let { str ->
                    str + " ".repeat(sizeIndex - str.length)
                }
            )

            append(
                "$available $name $args".let { str ->
                    str + " ".repeat(sizeInstruction - str.length)
                }
            )

            append(" | ")

            append(
                destinations.let { str ->
                    str + " ".repeat(sizeDestinations - str.length)
                }
            )

            appendBeforeAfter(
                before = usesBefore,
                after = usesAfter,
                sizeBefore = sizeUsesBefore,
                sizeAfter = sizeUsesAfter,
            )

            appendBeforeAfter(
                before = valuesBefore,
                after = valuesAfter,
                sizeBefore = sizeValuesBefore,
                sizeAfter = sizeValuesAfter,
            )

            appendBeforeAfter(
                before = versionsBefore,
                after = versionsAfter,
                sizeBefore = sizeVersionsBefore,
                sizeAfter = sizeVersionsAfter,
            )

            append("\n")
        }
    }

    if (has || out == null) {
        println(result)
    }

    return has
}