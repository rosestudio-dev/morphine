package ru.unit.morphine.assembly.compiler.ast.node

interface Visitor {

    fun visit(node: Node)
}