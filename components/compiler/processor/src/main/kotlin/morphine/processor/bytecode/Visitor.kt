package morphine.processor.bytecode

import com.google.devtools.ksp.KspExperimental
import com.google.devtools.ksp.getAnnotationsByType
import com.google.devtools.ksp.symbol.KSClassDeclaration
import com.google.devtools.ksp.symbol.KSNode
import com.google.devtools.ksp.visitor.KSEmptyVisitor
import com.squareup.kotlinpoet.ksp.toClassName

class Visitor<T : Annotation> : KSEmptyVisitor<ModelType<T>, Model>() {

    private companion object {

        fun getArgumentPackage(name: String) = "morphine.bytecode.Argument.$name"
    }

    override fun defaultHandler(node: KSNode, data: ModelType<T>) =
        throw Exception("Wrong feature scope visitor node ${node.origin.name}")

    @OptIn(KspExperimental::class)
    override fun visitClassDeclaration(classDeclaration: KSClassDeclaration, data: ModelType<T>): Model {
        val slots = classDeclaration.getAllProperties().filter { declaration ->
            declaration.type.resolve().toClassName().toString() == getArgumentPackage("Slot")
        }.map { declaration ->
            declaration.simpleName.getShortName()
        }

        val indexes = classDeclaration.getAllProperties().filter { declaration ->
            declaration.type.resolve().toClassName().toString() == getArgumentPackage("Index")
        }.map { declaration ->
            declaration.simpleName.getShortName()
        }

        val counts = classDeclaration.getAllProperties().filter { declaration ->
            declaration.type.resolve().toClassName().toString() == getArgumentPackage("Count")
        }.map { declaration ->
            declaration.simpleName.getShortName()
        }

        if (classDeclaration.getAnnotationsByType(data.kclass).toList().isEmpty()) {
            throw Exception("Unexpected behaviour")
        }

        return when (data) {
            ModelType.Clean -> Model.Clean(
                from = indexes.filter { declaration -> declaration == "from" }.single(),
                count = counts.filter { declaration -> declaration == "count" }.single(),
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )

            ModelType.Consumer -> Model.Consumer(
                sources = slots.toList(),
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )

            ModelType.Control -> Model.Control(
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )

            ModelType.Movement -> Model.Movement(
                source = slots.filterNot { declaration -> declaration == "destination" }.single(),
                destination = slots.filter { declaration -> declaration == "destination" }.single(),
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )

            ModelType.Processing -> Model.Processing(
                sources = slots.filterNot { declaration -> declaration == "destination" }.toList(),
                destination = slots.filter { declaration -> declaration == "destination" }.single(),
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )

            ModelType.Producer -> Model.Producer(
                destination = slots.single(),
                instruction = classDeclaration.toClassName(),
                file = classDeclaration.accept(FileVisitor(), Unit)!!
            )
        }
    }
}