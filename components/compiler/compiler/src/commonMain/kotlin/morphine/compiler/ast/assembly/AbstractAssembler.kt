package morphine.compiler.ast.assembly

import morphine.bytecode.Argument
import morphine.bytecode.Bytecode
import morphine.bytecode.Instruction
import morphine.bytecode.LineData
import morphine.bytecode.Value
import morphine.compiler.ast.assembly.exception.CompilerException
import morphine.compiler.ast.node.Accessible
import morphine.compiler.ast.node.Compiler
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.FunctionExpression
import morphine.compiler.ast.node.Node
import morphine.compiler.ast.node.Statement
import morphine.utils.UID

abstract class AbstractAssembler(
    private val optimize: Boolean
) : Compiler {

    var lineData: LineData? = null
        private set

    private val mainUID = UID()

    var function = Function(
        name = FunctionName.Normal("main"),
        uid = mainUID,
        arguments = emptyList(),
        statics = emptyList(),
        closureMode = FunctionExpression.ClosureMode.Manual(),
        isRecursive = false,
        parent = null
    )
        private set

    private val readyFunctions = mutableListOf<Function>()

    private val compileStack = mutableListOf<CompileInfo>()

    // region function

    fun functionEnter(
        name: FunctionName,
        arguments: List<String>,
        statics: List<String>,
        closureMode: FunctionExpression.ClosureMode,
        isRecursive: Boolean
    ) {
        function = Function(
            name = name,
            arguments = arguments,
            statics = statics,
            closureMode = closureMode,
            isRecursive = isRecursive,
            parent = function
        )
    }

    fun functionExit(): Function {
        val ready = function
        if (ready.parent == null) {
            throw CompilerException("Cannot exit from main function")
        } else {
            function = ready.parent
        }

        readyFunctions.add(ready)

        return ready
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

        val hasBreak = function.anchors.any { anchor -> anchor.marker == block.breakAnchorMarker }
        val hasContinue = function.anchors.any { anchor -> anchor.marker == block.continueAnchorMarker }

        if (!hasBreak || !hasContinue) {
            throw CompilerException("Break/Continue block corrupted")
        }
    }

    // endregion

    // region variable

    fun Function.defineVariable(name: String, isConst: Boolean): Access.Variable {
        val has = variables.levels.last().any { variable ->
            variable.value.name == name
        }

        return if (has) {
            throw CompilerException("'$name' variable cannot be defined, scope already contains it")
        } else {
            val variable = Scope.Slot.Variable(
                name = name,
                isConst = isConst
            )

            val index = variables.add(variable)

            Access.Variable(
                name = name,
                isConst = isConst,
                index = index
            )
        }
    }

    fun Function.accessVariable(name: String) =
        access(name) ?: throw CompilerException("'$name' variable cannot be accessed")

    private fun Function.access(name: String): Access? {
        variables.levels.reversed().forEach { scope ->
            val indexedVariable = scope.find { (_, variable) -> variable.name == name }

            if (indexedVariable != null) {
                return Access.Variable(
                    name = indexedVariable.value.name,
                    isConst = indexedVariable.value.isConst,
                    index = indexedVariable.index
                )
            }
        }

        statics.withIndex().find { (_, staticName) ->
            staticName == name
        }?.let { indexedArgument ->
            return Access.Static(
                name = indexedArgument.value,
                index = indexedArgument.index
            )
        }

        arguments.withIndex().find { (_, argName) ->
            argName == name
        }?.let { indexedArgument ->
            return Access.Argument(
                name = indexedArgument.value,
                index = indexedArgument.index
            )
        }

        if (isRecursive && name == (this.name as? FunctionName.Normal)?.value) {
            return Access.Recursion(name)
        }

        when (closureMode) {
            FunctionExpression.ClosureMode.Automatic -> automaticClosures
            is FunctionExpression.ClosureMode.Manual -> closureMode.list
        }.withIndex().find { (_, closureName) ->
            closureName.alias == name
        }?.let { indexedArgument ->
            return Access.Closure(
                name = indexedArgument.value.alias,
                index = indexedArgument.index
            )
        }

        when (closureMode) {
            FunctionExpression.ClosureMode.Automatic -> if (parent != null) {
                val closure = parent.access(name)
                if (closure != null) {
                    val created = FunctionExpression.Closure(
                        access = closure.name,
                        alias = closure.name,
                    )

                    val result = Access.Closure(
                        name = closure.name,
                        index = automaticClosures.size
                    )

                    automaticClosures.add(created)

                    return result
                }
            }

            is FunctionExpression.ClosureMode.Manual -> Unit
        }

        return null
    }

    // endregion

    // region bytecode

    fun bytecode() = Bytecode(
        mainFunction = mainUID,
        functions = (readyFunctions + function).map { function ->
            Bytecode.Function(
                uid = function.uid,
                name = when (function.name) {
                    FunctionName.Anonymous -> "anonymous${function.uid}"
                    is FunctionName.Normal -> function.name.value
                },
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
        val name: FunctionName,
        val uid: UID = UID(),
        val arguments: List<String>,
        val statics: List<String>,
        val closureMode: FunctionExpression.ClosureMode,
        val isRecursive: Boolean,
        val parent: Function?,
        val automaticClosures: MutableList<FunctionExpression.Closure> = mutableListOf(),
        val slots: MutableList<Scope.Slot> = mutableListOf(),
        val constants: MutableList<Value> = mutableListOf(),
        val instructions: MutableList<Instruction> = mutableListOf(),
        val anchors: MutableList<Anchor> = mutableListOf(),
    ) {

        val breakContinueAnchors = mutableListOf<BreakContinueAnchors>()
        val variables = Scope<Scope.Slot.Variable>(slots)
        val temporaries = Scope<Scope.Slot.Temporary>(slots)

        val breakAnchor
            get() = breakContinueAnchors.lastOrNull()?.breakAnchorMarker
                ?: throw CompilerException("Breakable block not found")

        val continueAnchor
            get() = breakContinueAnchors.lastOrNull()?.continueAnchorMarker
                ?: throw CompilerException("Continuable block not found")

        val closures
            get() = when (closureMode) {
                FunctionExpression.ClosureMode.Automatic -> automaticClosures
                is FunctionExpression.ClosureMode.Manual -> closureMode.list
            }
    }

    sealed interface FunctionName {

        data object Anonymous : FunctionName

        data class Normal(val value: String) : FunctionName
    }

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

        fun anchor(marker: AnchorMarker) {
            val anchor = function.anchors.find { anchor -> anchor.marker == marker }

            when {
                anchor == null -> function.anchors.add(
                    Anchor(
                        marker = marker,
                        position = function.instructions.size
                    )
                )

                anchor.position == null -> anchor.position = function.instructions.size

                else -> throw CompilerException("Anchor conflict")
            }
        }

        fun positionByAnchor(marker: AnchorMarker): Argument.Position {
            val anchorIndex = function.anchors.indexOfFirst { anchor ->
                anchor.marker == marker
            }

            return if (anchorIndex in function.anchors.indices) {
                Argument.Position(anchorIndex)
            } else {
                function.anchors.add(Anchor(marker = marker))
                Argument.Position(function.anchors.size - 1)
            }
        }

        fun Expression.evalWithResult(argumentSlot: Argument.Slot = slot()): Argument.Slot {
            compileStack.add(
                CompileInfo.EvalWithResult(
                    resultSlot = argumentSlot
                )
            )

            eval(this@AbstractAssembler)

            compileStack.removeLast()

            return argumentSlot
        }

        fun Expression.eval() {
            compileStack.add(CompileInfo.Eval)

            eval(this@AbstractAssembler)

            compileStack.removeLast()
        }

        fun Statement.exec() {
            compileStack.add(CompileInfo.Exec)

            exec(this@AbstractAssembler)

            compileStack.removeLast()
        }

        fun Accessible.set(slot: Argument.Slot) {
            compileStack.add(CompileInfo.Set)

            set(compiler = this@AbstractAssembler, slot = slot)

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
        val marker: AnchorMarker,
        var position: Int? = null
    )

    data class BreakContinueAnchors(
        val breakAnchorMarker: AnchorMarker = AnchorMarker(),
        val continueAnchorMarker: AnchorMarker = AnchorMarker(),
    )

    class AnchorMarker

    private sealed interface CompileInfo {

        data class EvalWithResult(
            val resultSlot: Argument.Slot
        ) : CompileInfo

        data object Eval : CompileInfo

        data object Set : CompileInfo
        data object Exec : CompileInfo
    }
}