package ru.unit.morphine.assembly.compiler.ast.compilerold

import ru.unit.morphine.assembly.bytecode.*
import ru.unit.morphine.assembly.compiler.ast.compilerold.exception.CompilerException
import ru.unit.morphine.assembly.compiler.ast.node.*
import java.util.*

abstract class AbstractCompiler(
    private val optimize: Boolean
) : Compiler {

    val function get() = functionStack.last()

    var lineData: LineData? = null
        private set

    private val mainUUID = UUID.randomUUID()

    private val mainFunction = Function(
        name = "main",
        uuid = mainUUID,
        arguments = emptyList(),
        statics = emptyList(),
    )

    private val functionStack = mutableListOf(mainFunction)

    private val readyFunctions = mutableListOf<Function>()

    private val compileStack = mutableListOf<CompileInfo>()

    // region function

    fun functionEnter(
        name: String?,
        arguments: List<String>,
        statics: List<String>
    ) {
        functionStack.add(
            Function(
                name = name,
                arguments = arguments,
                statics = statics
            )
        )
    }

    fun functionExit(): FunctionInfo {
        val function = functionStack.removeLast()
        readyFunctions.add(function)

        return FunctionInfo(
            uuid = function.uuid,
            closures = function.closures
        )
    }

    // endregion

    // region codegen

    fun Expression.codegenExpression(body: ExpressionCodegen.() -> Unit) {
        val savedLineData = lineData
        lineData = data.lineData

        function.temporaries.enter()
        body(ExpressionCodegen(this))
        function.temporaries.exit()

        lineData = savedLineData
    }

    fun Statement.codegenStatement(body: StatementCodegen.() -> Unit) {
        val savedLineData = lineData
        lineData = data.lineData

        function.temporaries.enter()
        body(StatementCodegen(this))
        function.temporaries.exit()

        lineData = savedLineData
    }

    fun Accessible.codegenAccessible(body: AccessibleCodegen.() -> Unit) {
        val savedLineData = lineData
        lineData = data.lineData

        function.temporaries.enter()
        body(AccessibleCodegen(this))
        function.temporaries.exit()

        lineData = savedLineData
    }

    // endregion

    // region anchors

    fun enterBreakContinue() {
        function.breakContinueAnchors.add(BreakContinueAnchors())
    }

    fun exitBreakContinue() {
        val block = function.breakContinueAnchors.removeLast()

        val hasBreak = function.anchors.any { anchor -> anchor.uuid == block.breakAnchorUUID }
        val hasContinue = function.anchors.any { anchor -> anchor.uuid == block.continueAnchorUUID }

        if (!hasBreak || !hasContinue) {
            throw CompilerException("Break/Continue block corrupted")
        }
    }

    // endregion

    // region variable

    fun defineVariable(name: String, isConst: Boolean): Access.Variable {
        val has = function.variables.levels.last().any { variable ->
            variable.value.name == name
        }

        return if (has) {
            throw CompilerException("'$name' variable cannot be defined, scope already contains it")
        } else {
            val variable = Scope.Slot.Variable(
                name = name,
                isConst = isConst
            )

            val index = function.variables.add(variable)

            Access.Variable(
                name = name,
                isConst = isConst,
                index = index
            )
        }
    }

    fun accessVariable(
        name: String
    ) = function.accessVariable(name) ?: run {
        val indexedFunction = functionStack.withIndex().reversed().find { (_, function) ->
            if (function == this.function) {
                false
            } else {
                val hasVariable = function.variables.accessVariable(name) != null
                val hasClosure = function.closures.any { closureName -> closureName == name }

                hasClosure || hasVariable
            }
        } ?: throw CompilerException("'$name' variable cannot be accessed")

        (indexedFunction.index + 1 until functionStack.size).forEach { index ->
            functionStack[index].closures.add(name)
        }

        return Access.Closure(
            name = name,
            index = function.closures.size - 1
        )
    }

    private fun Scope<Scope.Slot.Variable>.accessVariable(name: String): Access? {
        levels.reversed().forEach { scope ->
            val indexedVariable = scope.find { (_, variable) -> variable.name == name }

            if (indexedVariable != null) {
                return Access.Variable(
                    name = indexedVariable.value.name,
                    isConst = indexedVariable.value.isConst,
                    index = indexedVariable.index
                )
            }
        }

        return null
    }

    private fun Function.accessVariable(
        name: String
    ): Access? = variables.accessVariable(name) ?: run {
        arguments.withIndex().find { (_, argName) ->
            argName == name
        }?.let { indexedArgument ->
            return Access.Argument(
                name = indexedArgument.value,
                index = indexedArgument.index
            )
        }

        if (name == function.name) {
            return Access.Recursion(name)
        }

        closures.withIndex().find { (_, closureName) ->
            closureName == name
        }?.let { indexedArgument ->
            return Access.Closure(
                name = indexedArgument.value,
                index = indexedArgument.index
            )
        }

        statics.withIndex().find { (_, staticName) ->
            staticName == name
        }?.let { indexedArgument ->
            return Access.Static(
                name = indexedArgument.value,
                index = indexedArgument.index
            )
        }

        return null
    }

    // endregion

    // region bytecode

    fun bytecode() = Bytecode(
        mainFunction = mainUUID,
        functions = (readyFunctions + mainFunction).map { function ->
            Bytecode.Function(
                uuid = function.uuid,
                name = function.name ?: "anonymous'${function.uuid}",
                instructions = function.formatInstructions(),
                constants = function.constants,
                argumentsCount = function.arguments.size,
                staticsCount = function.statics.size,
                closuresCount = function.closures.size,
                slotsCount = function.slots.size,
                paramsCount = function.paramsCount(),
                optimize = optimize
            )
        }
    )

    private fun Function.resolveAnchor(position: Argument.Position) = Argument.Position(
        value = anchors.getOrNull(position.value)?.position ?: throw CompilerException("Anchor cannot be resolved")
    )

    private fun Function.formatInstructions() = instructions.map { instruction ->
        when (instruction) {
            is Instruction.Jump -> instruction.copy(
                position = resolveAnchor(instruction.position)
            )

            is Instruction.JumpIf -> instruction.copy(
                ifPosition = resolveAnchor(instruction.ifPosition),
                elsePosition = resolveAnchor(instruction.elsePosition)
            )

            else -> instruction
        }
    }

    private fun Function.paramsCount() = instructions
        .flatMap(Instruction::orderedArguments)
        .filterIsInstance<Argument.Param>()
        .maxOfOrNull(Argument.Param::value)?.plus(1) ?: 0

    // endregion

    data class Function(
        val name: String?,
        val uuid: UUID = UUID.randomUUID(),
        val arguments: List<String>,
        val statics: List<String>,
        val closures: MutableList<String> = mutableListOf(),
        val slots: MutableList<Scope.Slot> = mutableListOf(),
        val constants: MutableList<Value> = mutableListOf(),
        val instructions: MutableList<Instruction> = mutableListOf(),
        val anchors: MutableList<Anchor> = mutableListOf(),
    ) {

        val breakContinueAnchors = mutableListOf<BreakContinueAnchors>()
        val variables = Scope<Scope.Slot.Variable>(slots)
        val temporaries = Scope<Scope.Slot.Temporary>(slots)

        val breakAnchor
            get() = breakContinueAnchors.lastOrNull()?.breakAnchorUUID
                ?: throw CompilerException("Breakable block not found")

        val continueAnchor
            get() = breakContinueAnchors.lastOrNull()?.continueAnchorUUID
                ?: throw CompilerException("Continuable block not found")
    }

    data class FunctionInfo(
        val uuid: UUID,
        val closures: List<String>,
    )

    sealed interface Access {

        val name: String

        data class Variable(
            override val name: String,
            val index: Int,
            val isConst: Boolean
        ) : Access

        data class Argument(
            override val name: String,
            val index: Int,
        ) : Access

        data class Recursion(
            override val name: String
        ) : Access

        data class Closure(
            override val name: String,
            val index: Int,
        ) : Access

        data class Static(
            override val name: String,
            val index: Int,
        ) : Access
    }

    abstract inner class Codegen(private val node: Node) {

        fun constant(value: Value): Argument.Constant {
            val index = function.constants.indexOf(value)

            return if (index in function.constants.indices) {
                Argument.Constant(index)
            } else {
                function.constants.add(value)
                Argument.Constant(function.constants.size - 1)
            }
        }

        fun slot() = Argument.Slot(function.temporaries.add(Scope.Slot.Temporary))

        fun instruction(vararg instructions: Instruction?) {
            function.instructions.addAll(
                instructions.filterNotNull().onEach { instruction ->
                    instruction.lineData = node.data.lineData
                }
            )
        }

        fun anchor(uuid: UUID) {
            val anchor = function.anchors.find { anchor -> anchor.uuid == uuid }

            when {
                anchor == null -> function.anchors.add(
                    Anchor(
                        uuid = uuid,
                        position = function.instructions.size
                    )
                )

                anchor.position == null -> anchor.position = function.instructions.size

                else -> throw CompilerException("Anchor conflict")
            }
        }

        fun positionByAnchor(uuid: UUID): Argument.Position {
            val anchorIndex = function.anchors.indexOfFirst { anchor ->
                anchor.uuid == uuid
            }

            return if (anchorIndex in function.anchors.indices) {
                Argument.Position(anchorIndex)
            } else {
                function.anchors.add(Anchor(uuid = uuid))
                Argument.Position(function.anchors.size - 1)
            }
        }

        fun Expression.evalWithResult(argumentSlot: Argument.Slot = slot()): Argument.Slot {
            compileStack.add(
                CompileInfo.EvalWithResult(
                    resultSlot = argumentSlot
                )
            )

            eval(this@AbstractCompiler)

            compileStack.removeLast()

            return argumentSlot
        }

        fun Expression.eval() {
            compileStack.add(CompileInfo.Eval)

            eval(this@AbstractCompiler)

            compileStack.removeLast()
        }

        fun Statement.exec() {
            compileStack.add(CompileInfo.Exec)

            exec(this@AbstractCompiler)

            compileStack.removeLast()
        }

        fun Accessible.set(slot: Argument.Slot) {
            compileStack.add(CompileInfo.Set)

            set(compiler = this@AbstractCompiler, slot = slot)

            compileStack.removeLast()
        }
    }

    inner class ExpressionCodegen(node: Node) : Codegen(node) {

        fun result(withoutResult: () -> Unit = {}, withResult: (resultSlot: Argument.Slot) -> Unit) {
            val info = compileStack.last()

            if (info is CompileInfo.EvalWithResult) {
                withResult(info.resultSlot)
            } else {
                withoutResult()
            }
        }
    }

    inner class StatementCodegen(node: Node) : Codegen(node)

    inner class AccessibleCodegen(node: Node) : Codegen(node)

    data class Anchor(
        val uuid: UUID,
        var position: Int? = null
    )

    data class BreakContinueAnchors(
        val breakAnchorUUID: UUID = UUID.randomUUID(),
        val continueAnchorUUID: UUID = UUID.randomUUID(),
    )

    private sealed interface CompileInfo {

        data class EvalWithResult(
            val resultSlot: Argument.Slot
        ) : CompileInfo

        data object Eval : CompileInfo

        data object Set : CompileInfo
        data object Exec : CompileInfo
    }
}