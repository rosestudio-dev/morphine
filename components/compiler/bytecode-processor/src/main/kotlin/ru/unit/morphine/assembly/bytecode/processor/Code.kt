package ru.unit.morphine.assembly.bytecode.processor

import com.google.devtools.ksp.processing.SymbolProcessorEnvironment
import com.squareup.kotlinpoet.ClassName
import com.squareup.kotlinpoet.FileSpec
import com.squareup.kotlinpoet.FunSpec
import com.squareup.kotlinpoet.buildCodeBlock
import ru.unit.morphine.assembly.bytecode.processor.utils.PackageData

class Code(
    private val environment: SymbolProcessorEnvironment,
    private val packageData: PackageData,
) {

    private companion object {

        val abstractInstructionClassName = ClassName(
            "ru.unit.morphine.assembly.bytecode",
            "AbstractInstruction"
        )

        val instructionClassName = ClassName(
            "ru.unit.morphine.assembly.bytecode",
            "Instruction"
        )
    }

    fun generate(models: List<Model>) = FileSpec.builder(packageData.name, "AbstractInstructionExt").apply {
        models.forEach { model ->
            when (model.type) {
                Model.Type.Consumer -> {
                    assert(model.destination == null)
                    assert(model.sources.isNotEmpty())
                }

                Model.Type.Control -> {
                    assert(model.destination == null)
                    assert(model.sources.isEmpty())
                }

                Model.Type.Movement -> {
                    assert(model.destination != null)
                    assert(model.sources.size == 1)
                }

                Model.Type.Processing -> {
                    assert(model.destination != null)
                    assert(model.sources.isNotEmpty())
                }

                Model.Type.Producer -> {
                    assert(model.destination != null)
                    assert(model.sources.isEmpty())
                }
            }
        }

        addFunction(
            FunSpec.builder("abstract").apply {
                receiver(instructionClassName)
                returns(abstractInstructionClassName)

                buildCodeBlock {
                    beginControlFlow("return when (this)")

                    models.forEach { model ->
                        add("is %T -> ", model.instruction)

                        when (model.type) {
                            Model.Type.Consumer -> {
                                add("AbstractInstruction.Consumer(\n")
                                add("instruction = this,\n")
                                add("sources = listOf(${model.sources.joinToString()}),\n")
                                add(")\n")
                            }

                            Model.Type.Control -> {
                                add("AbstractInstruction.Control(\n")
                                add("instruction = this,\n")
                                add(")\n")
                            }

                            Model.Type.Movement -> {
                                add("AbstractInstruction.Movement(\n")
                                add("instruction = this,\n")
                                add("source = ${model.sources.single()},\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }

                            Model.Type.Processing -> {
                                add("AbstractInstruction.Processing(\n")
                                add("instruction = this,\n")
                                add("sources = listOf(${model.sources.joinToString()}),\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }

                            Model.Type.Producer -> {
                                add("AbstractInstruction.Producer(\n")
                                add("instruction = this,\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }
                        }

                        add("\n")
                    }

                    endControlFlow()
                }.also { codeBlock -> addCode(codeBlock) }
            }.build()
        )

        addFunction(
            FunSpec.builder("normalize").apply {
                receiver(abstractInstructionClassName)
                returns(instructionClassName)

                buildCodeBlock {
                    beginControlFlow("return when (val instruction = instruction)")

                    models.forEach { model ->
                        val name = when (model.type) {
                            Model.Type.Consumer -> "Consumer"
                            Model.Type.Control -> "Control"
                            Model.Type.Movement -> "Movement"
                            Model.Type.Processing -> "Processing"
                            Model.Type.Producer -> "Producer"
                        }

                        add("is %T -> (this as AbstractInstruction.$name).let { abstract -> \ninstruction.copy(\n", model.instruction)

                        when (model.type) {
                            Model.Type.Consumer -> {
                                model.sources.forEachIndexed { index, source ->
                                    add("$source = abstract.sources[$index],\n")
                                }
                            }

                            Model.Type.Control -> Unit

                            Model.Type.Movement -> {
                                add("${model.sources.single()} = abstract.source,\n")
                                add("${model.destination} = abstract.destination,\n")
                            }

                            Model.Type.Processing -> {
                                model.sources.forEachIndexed { index, source ->
                                    add("$source = abstract.sources[$index],\n")
                                }
                                add("${model.destination} = abstract.destination,\n")
                            }

                            Model.Type.Producer -> {
                                add("${model.destination} = abstract.destination,\n")
                            }
                        }
                        add(")\n}\n\n")
                    }

                    endControlFlow()
                }.also { codeBlock -> addCode(codeBlock) }
            }.build()
        )
    }.build()
}