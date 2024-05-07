package ru.unit.morphine.assembly.compiler.ast

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.compiler.ast.compilerold.CompilerInstance
import ru.unit.morphine.assembly.compiler.ast.compilerold.exception.CompilerException
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.ast.node.Visitor

data class Ast(val statement: Statement) {

    fun compile(optimize: Boolean): Bytecode {
        val compiler = CompilerInstance(optimize)

        return runCatching {
            statement.exec(compiler)
            compiler.bytecode()
        }.getOrElse { throwable ->
            if (throwable is CompilerException) {
                throw throwable.copy(lineData = compiler.lineData)
            } else {
                throw throwable
            }
        }
    }

    fun accept(visitor: Visitor) = statement.accept(visitor)
}