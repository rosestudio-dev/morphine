package ru.unit.morphine.assembly.compiler.ast.node

import ru.unit.morphine.assembly.bytecode.Argument

interface Compiler {

    fun eval(expression: Expression)
    fun set(accessible: Accessible, slot: Argument.Slot)
    fun exec(statement: Statement)
}