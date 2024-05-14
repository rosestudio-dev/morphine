package morphine.compiler.ast

import morphine.bytecode.Bytecode
import morphine.compiler.ast.assembly.AssemblerInstance
import morphine.compiler.ast.assembly.exception.CompilerException
import morphine.compiler.ast.node.Statement
import morphine.compiler.ast.node.Visitor

data class Ast(val statement: Statement) {

    fun assembly(optimize: Boolean): Bytecode {
        val assembler = AssemblerInstance(optimize)

        return runCatching {
            statement.exec(assembler)
            assembler.bytecode()
        }.getOrElse { throwable ->
            if (throwable is CompilerException) {
                throw throwable.copy(lineData = assembler.lineData)
            } else {
                throw throwable
            }
        }
    }

    fun accept(visitor: Visitor) = statement.accept(visitor)
}