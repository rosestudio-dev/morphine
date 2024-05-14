package morphine.compiler.ast.node

import morphine.bytecode.Argument

interface Compiler {

    fun eval(expression: Expression)
    fun set(accessible: Accessible, slot: Argument.Slot)
    fun exec(statement: Statement)
}