package ru.unit.morphine.assembly.compiler.ast.node

import ru.unit.morphine.assembly.bytecode.Value

data class ValueExpression(
    val value: Value,
    override val data: Node.Data
) : Expression