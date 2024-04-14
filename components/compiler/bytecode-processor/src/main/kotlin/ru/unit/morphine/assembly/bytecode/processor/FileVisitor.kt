package ru.unit.morphine.assembly.bytecode.processor

import com.google.devtools.ksp.symbol.KSFile
import com.google.devtools.ksp.symbol.KSNode
import com.google.devtools.ksp.visitor.KSDefaultVisitor

class FileVisitor : KSDefaultVisitor<Unit, KSFile?>() {

    override fun defaultHandler(node: KSNode, data: Unit): KSFile? = node.parent?.accept(this, Unit)

    override fun visitFile(file: KSFile, data: Unit) = file
}