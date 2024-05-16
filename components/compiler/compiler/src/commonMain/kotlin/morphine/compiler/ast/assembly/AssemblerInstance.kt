package morphine.compiler.ast.assembly

import morphine.bytecode.Argument
import morphine.bytecode.Instruction
import morphine.bytecode.Value
import morphine.compiler.ast.assembly.exception.CompilerException
import morphine.compiler.ast.node.AccessAccessible
import morphine.compiler.ast.node.Accessible
import morphine.compiler.ast.node.AssigmentStatement
import morphine.compiler.ast.node.AssignMethod
import morphine.compiler.ast.node.BinaryExpression
import morphine.compiler.ast.node.BlockExpression
import morphine.compiler.ast.node.BlockStatement
import morphine.compiler.ast.node.BreakStatement
import morphine.compiler.ast.node.CallExpression
import morphine.compiler.ast.node.CallSelfExpression
import morphine.compiler.ast.node.ContinueStatement
import morphine.compiler.ast.node.DeclarationStatement
import morphine.compiler.ast.node.DoWhileStatement
import morphine.compiler.ast.node.EmptyStatement
import morphine.compiler.ast.node.EnvExpression
import morphine.compiler.ast.node.EvalStatement
import morphine.compiler.ast.node.Expression
import morphine.compiler.ast.node.ForStatement
import morphine.compiler.ast.node.FunctionExpression
import morphine.compiler.ast.node.IfExpression
import morphine.compiler.ast.node.IfStatement
import morphine.compiler.ast.node.IncrementExpression
import morphine.compiler.ast.node.IteratorStatement
import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.ast.node.SelfExpression
import morphine.compiler.ast.node.Statement
import morphine.compiler.ast.node.TableExpression
import morphine.compiler.ast.node.UnaryExpression
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.ast.node.VariableAccessible
import morphine.compiler.ast.node.VectorExpression
import morphine.compiler.ast.node.WhileStatement
import morphine.compiler.ast.node.YieldStatement

class AssemblerInstance(optimize: Boolean) : AbstractAssembler(optimize) {

    override fun eval(expression: Expression) = when (expression) {
        is AccessAccessible -> expression.eval()
        is VariableAccessible -> expression.eval()
        is BinaryExpression -> expression.eval()
        is BlockExpression -> expression.eval()
        is CallExpression -> expression.eval()
        is CallSelfExpression -> expression.eval()
        is EnvExpression -> expression.eval()
        is FunctionExpression -> expression.eval()
        is IfExpression -> expression.eval()
        is IncrementExpression -> expression.eval()
        is SelfExpression -> expression.eval()
        is TableExpression -> expression.eval()
        is UnaryExpression -> expression.eval()
        is ValueExpression -> expression.eval()
        is VectorExpression -> expression.eval()
    }

    override fun set(accessible: Accessible, slot: Argument.Slot) = when (accessible) {
        is AccessAccessible -> accessible.set(slot)
        is VariableAccessible -> accessible.set(slot)
    }

    override fun exec(statement: Statement) = when (statement) {
        is AssigmentStatement -> statement.exec()
        is BlockStatement -> statement.exec()
        is BreakStatement -> statement.exec()
        is ContinueStatement -> statement.exec()
        is DeclarationStatement -> statement.exec()
        is DoWhileStatement -> statement.exec()
        is EmptyStatement -> Unit
        is EvalStatement -> statement.exec()
        is ForStatement -> statement.exec()
        is IfStatement -> statement.exec()
        is ReturnStatement -> statement.exec()
        is WhileStatement -> statement.exec()
        is IteratorStatement -> statement.exec()
        is YieldStatement -> statement.exec()
    }

    private fun ForStatement.exec() = codegenStatement {
        enterBreakContinue()

        function.variables.enter()

        initial.exec()

        anchor(function.continueAnchor)
        val condition = condition.evalWithResult()
        val blockAnchorMarker = AnchorMarker()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(blockAnchorMarker),
                elsePosition = positionByAnchor(function.breakAnchor)
            )
        )

        anchor(blockAnchorMarker)
        statement.exec()

        iterator.exec()

        instruction(
            Instruction.Jump(
                position = positionByAnchor(function.continueAnchor)
            )
        )

        function.variables.exit()

        anchor(function.breakAnchor)

        exitBreakContinue()
    }

    private fun WhileStatement.exec() = codegenStatement {
        enterBreakContinue()

        anchor(function.continueAnchor)
        val condition = condition.evalWithResult()
        val blockAnchorMarker = AnchorMarker()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(blockAnchorMarker),
                elsePosition = positionByAnchor(function.breakAnchor)
            )
        )

        function.variables.enter()

        anchor(blockAnchorMarker)
        statement.exec()

        instruction(
            Instruction.Jump(
                position = positionByAnchor(function.continueAnchor)
            )
        )

        function.variables.exit()

        anchor(function.breakAnchor)

        exitBreakContinue()
    }

    private fun IteratorStatement.exec() = codegenStatement {
        enterBreakContinue()

        function.variables.enter()

        val iterator = iterable.evalWithResult()

        instruction(
            Instruction.Iterator(
                container = iterator,
                destination = iterator
            ),
            Instruction.IteratorInit(
                iterator = iterator
            )
        )

        anchor(function.continueAnchor)
        val condition = slot()
        instruction(
            Instruction.IteratorHas(
                iterator = iterator,
                destination = condition
            )
        )

        val blockAnchorMarker = AnchorMarker()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(blockAnchorMarker),
                elsePosition = positionByAnchor(function.breakAnchor)
            )
        )

        anchor(blockAnchorMarker)

        val next = slot()

        instruction(
            Instruction.IteratorNext(
                iterator = iterator,
                destination = next
            )
        )

        declaration(
            method = method,
            isMutable = false,
            source = next
        )

        function.variables.enter()
        statement.exec()
        function.variables.exit()

        instruction(
            Instruction.Jump(
                position = positionByAnchor(function.continueAnchor)
            )
        )

        function.variables.exit()

        anchor(function.breakAnchor)

        exitBreakContinue()
    }

    private fun DoWhileStatement.exec() = codegenStatement {
        enterBreakContinue()

        anchor(function.continueAnchor)

        function.variables.enter()

        statement.exec()

        val condition = condition.evalWithResult()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(function.continueAnchor),
                elsePosition = positionByAnchor(function.breakAnchor)
            )
        )

        function.variables.exit()

        anchor(function.breakAnchor)

        exitBreakContinue()
    }

    private fun BreakStatement.exec() = codegenStatement {
        instruction(
            Instruction.Jump(
                position = positionByAnchor(function.breakAnchor)
            )
        )
    }

    private fun ContinueStatement.exec() = codegenStatement {
        instruction(
            Instruction.Jump(
                position = positionByAnchor(function.continueAnchor)
            )
        )
    }

    private fun IfExpression.eval() = codegenExpression {
        val condition = condition.evalWithResult()
        val ifAnchorMarker = AnchorMarker()
        val elseAnchorMarker = AnchorMarker()
        val endAnchorMarker = AnchorMarker()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(ifAnchorMarker),
                elsePosition = positionByAnchor(elseAnchorMarker),
            )
        )

        fun produce(slot: Argument.Slot) {
            anchor(ifAnchorMarker)
            ifExpression.evalWithResult(slot)
            instruction(
                Instruction.Jump(
                    position = positionByAnchor(endAnchorMarker)
                )
            )

            anchor(elseAnchorMarker)
            elseExpression.evalWithResult(slot)

            anchor(endAnchorMarker)
        }

        result(
            withResult = { resultSlot ->
                produce(resultSlot)
            },
            withoutResult = {
                produce(slot())
            }
        )
    }

    private fun IfStatement.exec() = codegenStatement {
        val condition = condition.evalWithResult()
        val ifAnchorMarker = AnchorMarker()
        val elseAnchorMarker = AnchorMarker()
        val endAnchorMarker = AnchorMarker()

        instruction(
            Instruction.JumpIf(
                source = condition,
                ifPosition = positionByAnchor(ifAnchorMarker),
                elsePosition = positionByAnchor(elseAnchorMarker),
            )
        )

        anchor(ifAnchorMarker)
        ifStatement.exec()
        instruction(
            Instruction.Jump(
                position = positionByAnchor(endAnchorMarker)
            )
        )

        anchor(elseAnchorMarker)
        elseStatement.exec()

        anchor(endAnchorMarker)
    }

    private fun IncrementExpression.eval() = codegenExpression {
        val value = accessible.evalWithResult()

        fun produce(slot: Argument.Slot) {
            instruction(
                Instruction.Load(
                    constant = constant(Value.Integer(1)),
                    destination = slot
                ),
                when (type) {
                    IncrementExpression.Type.INCREMENT -> Instruction.Add(
                        sourceLeft = value,
                        sourceRight = slot,
                        destination = slot
                    )

                    IncrementExpression.Type.DECREMENT -> Instruction.Sub(
                        sourceLeft = value,
                        sourceRight = slot,
                        destination = slot
                    )
                }
            )

            accessible.set(slot)
        }

        result(
            withResult = { resultSlot ->
                produce(resultSlot)

                if (isPostfix) {
                    instruction(
                        Instruction.Move(
                            source = value,
                            destination = resultSlot
                        )
                    )
                }
            },
            withoutResult = {
                produce(slot())
            }
        )
    }

    private fun ReturnStatement.exec() = codegenStatement {
        instruction(
            Instruction.Leave(
                source = expression.evalWithResult()
            )
        )
    }

    private fun YieldStatement.exec() = codegenStatement {
        instruction(Instruction.Yield())
    }

    private fun AssigmentStatement.exec() = codegenStatement {
        when (method) {
            is AssignMethod.Decompose -> {
                val entries = method.entries.ifEmpty {
                    throw CompilerException("Empty assigment")
                }

                val source = expression.evalWithResult()
                val slot = slot()

                entries.forEach { entry ->
                    entry.key.evalWithResult(slot)

                    instruction(
                        Instruction.Get(
                            container = source,
                            keySource = slot,
                            destination = slot
                        )
                    )

                    entry.value.set(slot)
                }
            }

            is AssignMethod.Single -> {
                val source = if (binaryType != null) {
                    BinaryExpression(
                        type = binaryType,
                        expressionA = method.entry,
                        expressionB = expression,
                        data = data
                    )
                } else {
                    expression
                }.evalWithResult()

                method.entry.set(source)
            }
        }
    }

    private fun DeclarationStatement.exec() = codegenStatement {
        declaration(
            method = method,
            isMutable = isMutable,
            source = expression.evalWithResult()
        )
    }

    private fun AccessAccessible.eval() = codegenExpression {
        val container = container.evalWithResult()
        val key = key.evalWithResult()

        fun produce(slot: Argument.Slot) = Instruction.Get(
            container = container,
            keySource = key,
            destination = slot
        )

        result(
            withResult = { resultSlot ->
                instruction(produce(resultSlot))
            },
            withoutResult = {
                instruction(produce(slot()))
            }
        )
    }

    private fun AccessAccessible.set(slot: Argument.Slot) = codegenAccessible {
        val container = container.evalWithResult()
        val key = key.evalWithResult()

        instruction(
            Instruction.Set(
                source = slot,
                keySource = key,
                container = container
            )
        )
    }

    private fun CallExpression.eval() = codegenExpression {
        val callable = expression.evalWithResult()

        val slots = arguments.map { argument ->
            argument.evalWithResult()
        }

        slots.forEachIndexed { index, slot ->
            instruction(
                Instruction.Param(
                    source = slot,
                    param = Argument.Param(index)
                )
            )
        }

        instruction(
            Instruction.Call(
                source = callable,
                count = Argument.Count(arguments.size)
            )
        )

        result { resultSlot ->
            instruction(
                Instruction.Result(
                    destination = resultSlot,
                )
            )
        }
    }

    private fun CallSelfExpression.eval() = codegenExpression {
        val self = self.evalWithResult()
        val callable = callable.evalWithResult()

        if (extractCallable) {
            instruction(
                Instruction.Get(
                    container = self,
                    keySource = callable,
                    destination = callable
                )
            )
        }

        val slots = arguments.map { argument ->
            argument.evalWithResult()
        }

        slots.forEachIndexed { index, slot ->
            instruction(
                Instruction.Param(
                    source = slot,
                    param = Argument.Param(index)
                )
            )
        }

        instruction(
            Instruction.SCall(
                source = callable,
                selfSource = self,
                count = Argument.Count(arguments.size)
            )
        )

        result { resultSlot ->
            instruction(
                Instruction.Result(
                    destination = resultSlot,
                )
            )
        }
    }

    private fun EnvExpression.eval() = codegenExpression {
        result { resultSlot ->
            instruction(Instruction.Environment(resultSlot))
        }
    }

    private fun SelfExpression.eval() = codegenExpression {
        result { resultSlot ->
            instruction(Instruction.Self(resultSlot))
        }
    }

    private fun TableExpression.eval() = codegenExpression {
        result(
            withResult = { resultSlot ->
                instruction(Instruction.Table(resultSlot))

                val keySlot = slot()
                val valueSlot = slot()
                elements.forEach { (key, value) ->
                    key.evalWithResult(keySlot)
                    value.evalWithResult(valueSlot)

                    instruction(
                        Instruction.Set(
                            source = valueSlot,
                            keySource = keySlot,
                            container = resultSlot
                        )
                    )
                }
            },
            withoutResult = {
                elements.forEach { (key, value) ->
                    key.eval()
                    value.eval()
                }
            }
        )
    }

    private fun VectorExpression.eval() = codegenExpression {
        result(
            withResult = { resultSlot ->
                instruction(
                    Instruction.Vector(
                        destination = resultSlot,
                        count = Argument.Count(elements.size)
                    )
                )

                val keySlot = slot()
                val valueSlot = slot()
                elements.forEachIndexed { index, value ->
                    instruction(
                        Instruction.Load(
                            constant = constant(Value.Integer(index)),
                            destination = keySlot
                        )
                    )
                    value.evalWithResult(valueSlot)

                    instruction(
                        Instruction.Set(
                            source = valueSlot,
                            keySource = keySlot,
                            container = resultSlot
                        )
                    )
                }
            },
            withoutResult = {
                elements.forEach { value ->
                    value.eval()
                }
            }
        )
    }

    private fun UnaryExpression.eval() = codegenExpression {
        val a = expression.evalWithResult()

        fun produce(slot: Argument.Slot) = when (type) {
            UnaryExpression.Type.NEGATE -> Instruction.Negative(
                source = a,
                destination = slot
            )

            UnaryExpression.Type.NOT -> Instruction.Not(
                source = a,
                destination = slot
            )

            UnaryExpression.Type.TYPE -> Instruction.Type(
                source = a,
                destination = slot
            )

            UnaryExpression.Type.LEN -> Instruction.Length(
                source = a,
                destination = slot
            )

            UnaryExpression.Type.REF -> Instruction.Ref(
                source = a,
                destination = slot
            )

            UnaryExpression.Type.DEREF -> Instruction.Deref(
                source = a,
                destination = slot
            )
        }

        result(
            withoutResult = {
                instruction(produce(slot()))
            },
            withResult = { resultSlot ->
                instruction(produce(resultSlot))
            }
        )
    }

    private fun BinaryExpression.eval() = codegenExpression {
        fun produce(slot: Argument.Slot) {
            val exitAnchorMarker = AnchorMarker()
            val bAnchorMarker = AnchorMarker()

            val a = expressionA.evalWithResult(slot)

            when (type) {
                BinaryExpression.Type.OR -> {
                    instruction(
                        Instruction.JumpIf(
                            source = a,
                            ifPosition = positionByAnchor(exitAnchorMarker),
                            elsePosition = positionByAnchor(bAnchorMarker)
                        )
                    )
                }

                BinaryExpression.Type.AND -> {
                    instruction(
                        Instruction.JumpIf(
                            source = a,
                            ifPosition = positionByAnchor(bAnchorMarker),
                            elsePosition = positionByAnchor(exitAnchorMarker)
                        )
                    )
                }

                else -> Unit
            }

            anchor(bAnchorMarker)

            val b = expressionB.evalWithResult()

            val binaryInstruction = when (type) {
                BinaryExpression.Type.ADD -> Instruction.Add(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.SUB -> Instruction.Sub(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.MUL -> Instruction.Mul(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.DIV -> Instruction.Div(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.MOD -> Instruction.Mod(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.EQUALS -> Instruction.Equal(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.LESS -> Instruction.Less(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.LESS_EQUALS -> Instruction.LessEqual(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.OR -> Instruction.Or(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.AND -> Instruction.And(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )

                BinaryExpression.Type.CONCAT -> Instruction.Concat(
                    sourceLeft = a,
                    sourceRight = b,
                    destination = slot
                )
            }

            instruction(binaryInstruction)

            anchor(exitAnchorMarker)
        }

        result(
            withoutResult = {
                produce(slot())
            },
            withResult = { resultSlot ->
                produce(resultSlot)
            }
        )
    }

    private fun EvalStatement.exec() = codegenStatement {
        expression.eval()
    }

    private fun BlockStatement.exec() = codegenStatement {
        function.variables.enter()

        statements.forEach { statement ->
            statement.exec()
        }

        function.variables.exit()
    }

    private fun BlockExpression.eval() = codegenExpression {
        function.variables.enter()

        statements.forEach { statement ->
            statement.exec()
        }

        result(
            withResult = { resultSlot ->
                expression.evalWithResult(resultSlot)
            },
            withoutResult = {
                expression.eval()
            }
        )

        function.variables.exit()
    }

    private fun FunctionExpression.eval() = codegenExpression {
        val name = if (name == null) {
            FunctionName.Anonymous
        } else {
            FunctionName.Normal(name)
        }

        arguments.forEach { argument ->
            if (arguments.count { arg -> arg == argument } > 1) {
                throw CompilerException("Argument $argument is exists")
            }
        }

        closures.forEach { closure ->
            if (closures.count { c -> c.alias == closure.alias } > 1) {
                throw CompilerException("Closure variable ${closure.alias} is exists")
            }
        }

        statics.forEach { static ->
            if (statics.count { s -> s == static } > 1) {
                throw CompilerException("Static variable $static is exists")
            }
        }

        functionEnter(
            name = name,
            arguments = arguments,
            statics = statics,
            closures = closures,
            isRecursive = isRecursive
        )

        function.variables.enter()

        statement.exec()

        function.variables.exit()

        val uid = functionExit()

        result { resultSlot ->
            instruction(
                Instruction.Load(
                    constant = constant(Value.Function(uid)),
                    destination = resultSlot
                )
            )

            if (closures.isNotEmpty()) {
                val slots = closures.map { slot() }

                closures.mapIndexed { index, name ->
                    val slot = VariableAccessible(
                        name = name.access,
                        data = data
                    ).evalWithResult(slots[index])

                    instruction(
                        Instruction.Param(
                            param = Argument.Param(index),
                            source = slot
                        )
                    )
                }

                instruction(
                    Instruction.Closure(
                        source = resultSlot,
                        count = Argument.Count(closures.size),
                        destination = resultSlot
                    ),
                )
            }
        }
    }

    private fun ValueExpression.eval() = codegenExpression {
        result { resultSlot ->
            instruction(
                Instruction.Load(
                    constant = constant(value),
                    destination = resultSlot
                )
            )
        }
    }

    private fun VariableAccessible.eval() = codegenExpression {
        val variable = accessVariable(name)

        result { resultSlot ->
            val access = when (variable) {
                is Access.Closure,
                is Access.Static -> Instruction.Recursion(
                    destination = resultSlot
                )

                else -> null
            }

            val instruction = when (variable) {
                is Access.Argument -> Instruction.Arg(
                    arg = Argument.Arg(variable.index),
                    destination = resultSlot
                )

                is Access.Closure -> Instruction.GetClosure(
                    closure = resultSlot,
                    index = Argument.Index(variable.index),
                    destination = resultSlot
                )

                is Access.Recursion -> Instruction.Recursion(
                    destination = resultSlot
                )

                is Access.Static -> Instruction.GetStatic(
                    callable = resultSlot,
                    index = Argument.Index(variable.index),
                    destination = resultSlot
                )

                is Access.Variable -> Instruction.Move(
                    source = Argument.Slot(variable.index),
                    destination = resultSlot
                )
            }

            instruction(access, instruction)
        }
    }

    private fun VariableAccessible.set(slot: Argument.Slot) = codegenAccessible {
        val variable = accessVariable(name)

        instruction(
            *variableSet(
                name = name,
                variable = variable,
                slot = slot
            ).toTypedArray()
        )
    }

    private fun Codegen.variableSet(
        name: String,
        variable: Access,
        slot: Argument.Slot
    ) = when (variable) {
        is Access.Closure -> {
            val temp = slot()
            listOf(
                Instruction.Recursion(
                    destination = temp
                ),
                Instruction.SetClosure(
                    closure = temp,
                    index = Argument.Index(variable.index),
                    source = slot
                )
            )
        }

        is Access.Static -> {
            val temp = slot()
            listOf(
                Instruction.Recursion(
                    destination = temp
                ),
                Instruction.SetStatic(
                    callable = temp,
                    index = Argument.Index(variable.index),
                    source = slot
                )
            )
        }

        is Access.Variable -> listOf(
            Instruction.Move(
                source = slot,
                destination = Argument.Slot(variable.index)
            )
        )

        is Access.Argument -> throw CompilerException("Argument '$name' cannot be modified")

        is Access.Recursion -> throw CompilerException("Recursion '$name' cannot be modified")
    }

    private fun AbstractAssembler.StatementCodegen.declaration(
        method: AssignMethod<String>,
        isMutable: Boolean,
        source: Argument.Slot
    ) {
        when (method) {
            is AssignMethod.Decompose -> {
                val entries = method.entries.ifEmpty {
                    throw CompilerException("Empty declaration")
                }

                val slot = slot()

                entries.forEach { entry ->
                    entry.key.evalWithResult(slot)

                    instruction(
                        Instruction.Get(
                            container = source,
                            keySource = slot,
                            destination = slot
                        )
                    )

                    val variable = defineVariable(
                        name = entry.value,
                        isConst = !isMutable,
                    )

                    instruction(
                        *variableSet(
                            name = entry.value,
                            variable = variable,
                            slot = slot
                        ).toTypedArray()
                    )
                }
            }

            is AssignMethod.Single -> {
                val variable = defineVariable(
                    name = method.entry,
                    isConst = !isMutable
                )

                instruction(
                    *variableSet(
                        name = method.entry,
                        variable = variable,
                        slot = source
                    ).toTypedArray()
                )
            }
        }
    }
}