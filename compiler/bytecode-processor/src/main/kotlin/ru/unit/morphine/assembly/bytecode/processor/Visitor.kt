package ru.unit.morphine.assembly.bytecode.processor

import com.google.devtools.ksp.KspExperimental
import com.google.devtools.ksp.getAnnotationsByType
import com.google.devtools.ksp.symbol.KSClassDeclaration
import com.google.devtools.ksp.symbol.KSNode
import com.google.devtools.ksp.visitor.KSEmptyVisitor
import com.squareup.kotlinpoet.ksp.toClassName

class Visitor<T : Annotation> : KSEmptyVisitor<Model.Type<T>, Model>() {

    private companion object {

        const val ARGUMENT_SLOT_PACKAGE = "ru.unit.morphine.assembly.bytecode.Argument.Slot"
    }

    override fun defaultHandler(node: KSNode, data: Model.Type<T>) =
        throw Exception("Wrong feature scope visitor node ${node.origin.name}")

    @OptIn(KspExperimental::class)
    override fun visitClassDeclaration(classDeclaration: KSClassDeclaration, data: Model.Type<T>): Model {
        val slots = classDeclaration.getAllProperties().filter { declaration ->
            declaration.type.resolve().toClassName().toString() == ARGUMENT_SLOT_PACKAGE
        }.map { declaration ->
            declaration.simpleName.getShortName()
        }

        if (classDeclaration.getAnnotationsByType(data.kclass).toList().isEmpty()) {
            throw Exception("Unexpected behaviour")
        }

        return Model(
            type = data,
            instruction = classDeclaration.toClassName(),
            destination = slots.filter { declaration -> declaration == "destination" }.singleOrNull(),
            sources = slots.filterNot { declaration -> declaration == "destination" }.toList(),
        )
    }
}