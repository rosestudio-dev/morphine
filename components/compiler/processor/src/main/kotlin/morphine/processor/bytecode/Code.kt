package morphine.processor.bytecode

import com.google.devtools.ksp.processing.SymbolProcessorEnvironment
import com.squareup.kotlinpoet.ClassName
import com.squareup.kotlinpoet.FileSpec
import com.squareup.kotlinpoet.FunSpec
import com.squareup.kotlinpoet.buildCodeBlock

class Code(
    private val environment: SymbolProcessorEnvironment,
    private val packageData: PackageData,
) {

    private companion object {

        val abstractInstructionClassName = ClassName(
            "morphine.bytecode",
            "AbstractInstruction"
        )

        val instructionClassName = ClassName(
            "morphine.bytecode",
            "Instruction"
        )
    }

    fun generate(models: List<Model>) = FileSpec.builder(packageData.name, "AbstractInstructionExt").apply {
        addFunction(
            FunSpec.builder("abstract").apply {
                receiver(instructionClassName)
                returns(abstractInstructionClassName)

                buildCodeBlock {
                    beginControlFlow("return when (this)")

                    models.forEach { model ->
                        add("is %T -> ", model.instruction)

                        when (model) {
                            is Model.Consumer -> {
                                add("AbstractInstruction.Consumer(\n")
                                add("instruction = this,\n")
                                add("sources = listOf(${model.sources.joinToString()}),\n")
                                add(")\n")
                            }

                            is Model.Control -> {
                                add("AbstractInstruction.Control(\n")
                                add("instruction = this,\n")
                                add(")\n")
                            }

                            is Model.Movement -> {
                                add("AbstractInstruction.Movement(\n")
                                add("instruction = this,\n")
                                add("source = ${model.source},\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }

                            is Model.Processing -> {
                                add("AbstractInstruction.Processing(\n")
                                add("instruction = this,\n")
                                add("sources = listOf(${model.sources.joinToString()}),\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }

                            is Model.Producer -> {
                                add("AbstractInstruction.Producer(\n")
                                add("instruction = this,\n")
                                add("destination = ${model.destination},\n")
                                add(")\n")
                            }

                            is Model.Clean -> {
                                add("AbstractInstruction.Clean(\n")
                                add("instruction = this,\n")
                                add("from = ${model.from},\n")
                                add("count = ${model.count},\n")
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
                        val name = when (model) {
                            is Model.Consumer -> "Consumer"
                            is Model.Control -> "Control"
                            is Model.Movement -> "Movement"
                            is Model.Processing -> "Processing"
                            is Model.Producer -> "Producer"
                            is Model.Clean -> "Clean"
                        }

                        add(
                            "is %T -> (this as AbstractInstruction.$name).let { abstract -> \ninstruction.copy(\n",
                            model.instruction
                        )

                        when (model) {
                            is Model.Consumer -> {
                                model.sources.forEachIndexed { index, source ->
                                    add("$source = abstract.sources[$index],\n")
                                }
                            }

                            is Model.Control -> Unit

                            is Model.Movement -> {
                                add("${model.source} = abstract.source,\n")
                                add("${model.destination} = abstract.destination,\n")
                            }

                            is Model.Processing -> {
                                model.sources.forEachIndexed { index, source ->
                                    add("$source = abstract.sources[$index],\n")
                                }
                                add("${model.destination} = abstract.destination,\n")
                            }

                            is Model.Producer -> {
                                add("${model.destination} = abstract.destination,\n")
                            }

                            is Model.Clean -> {
                                add("${model.from} = abstract.from,\n")
                                add("${model.count} = abstract.count,\n")
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