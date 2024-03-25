package ru.unit.morphine.assembly.compiler.ast.node

sealed interface Statement : Node {

    fun <T : Compiler> exec(compiler: T) = compiler.exec(this)
}