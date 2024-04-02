package ru.unit.morphine.assembly.compiler.ast.visitor

import ru.unit.morphine.assembly.compiler.ast.node.AccessAccessible
import ru.unit.morphine.assembly.compiler.ast.node.AssigmentStatement
import ru.unit.morphine.assembly.compiler.ast.node.AssignMethod
import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.BlockStatement
import ru.unit.morphine.assembly.compiler.ast.node.BreakStatement
import ru.unit.morphine.assembly.compiler.ast.node.CallExpression
import ru.unit.morphine.assembly.compiler.ast.node.CallSelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.ContinueStatement
import ru.unit.morphine.assembly.compiler.ast.node.DeclarationStatement
import ru.unit.morphine.assembly.compiler.ast.node.DoWhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.EmptyStatement
import ru.unit.morphine.assembly.compiler.ast.node.EnvExpression
import ru.unit.morphine.assembly.compiler.ast.node.EvalStatement
import ru.unit.morphine.assembly.compiler.ast.node.ForStatement
import ru.unit.morphine.assembly.compiler.ast.node.FunctionExpression
import ru.unit.morphine.assembly.compiler.ast.node.IfStatement
import ru.unit.morphine.assembly.compiler.ast.node.IncDecExpression
import ru.unit.morphine.assembly.compiler.ast.node.IteratorStatement
import ru.unit.morphine.assembly.compiler.ast.node.Node
import ru.unit.morphine.assembly.compiler.ast.node.ReturnStatement
import ru.unit.morphine.assembly.compiler.ast.node.SelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.TableExpression
import ru.unit.morphine.assembly.compiler.ast.node.UnaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.ValueExpression
import ru.unit.morphine.assembly.compiler.ast.node.VariableAccessible
import ru.unit.morphine.assembly.compiler.ast.node.WhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.YieldStatement

abstract class SimpleVisitor : AbstractVisitor() {

    override fun visit(node: BinaryExpression) {
        node.expressionA.accept()
        node.expressionB.accept()
    }

    override fun visit(node: BlockStatement) {
        node.statements.accept()
    }

    override fun visit(node: CallExpression) {
        node.expression.accept()
        node.arguments.accept()
    }

    override fun visit(node: CallSelfExpression) {
        node.self.accept()
        node.callable.accept()
        node.arguments.accept()
    }

    override fun visit(node: EnvExpression) = Unit

    override fun visit(node: FunctionExpression) {
        node.statement.accept()
    }

    override fun visit(node: IfStatement) {
        node.condition.accept()
        node.ifStatement.accept()
        node.elseStatement.accept()
    }

    override fun visit(node: IncDecExpression) {
        node.accessible.accept()
    }

    override fun visit(node: SelfExpression) = Unit

    override fun visit(node: TableExpression) {
        node.elements.map(TableExpression.Element::key).accept()
        node.elements.map(TableExpression.Element::value).accept()
    }

    override fun visit(node: UnaryExpression) {
        node.expression.accept()
    }

    override fun visit(node: ValueExpression) = Unit

    override fun visit(node: AssigmentStatement) {
        when (node.method) {
            is AssignMethod.Decompose -> {
                node.method.entries.map { entry -> entry.value }.accept()
                node.method.entries.map { entry -> entry.key }.accept()
            }

            is AssignMethod.Single -> {
                node.method.entry.accept()
            }
        }

        node.expression.accept()
    }

    override fun visit(node: BreakStatement) = Unit

    override fun visit(node: ContinueStatement) = Unit

    override fun visit(node: DeclarationStatement) {
        when (node.method) {
            is AssignMethod.Decompose -> {
                node.method.entries.map { entry -> entry.key }.accept()
            }

            is AssignMethod.Single -> Unit
        }

        node.expression.accept()
    }

    override fun visit(node: DoWhileStatement) {
        node.condition.accept()
        node.statement.accept()
    }

    override fun visit(node: EmptyStatement) = Unit

    override fun visit(node: EvalStatement) {
        node.expression.accept()
    }

    override fun visit(node: ForStatement) {
        node.initial.accept()
        node.condition.accept()
        node.iterator.accept()
        node.statement.accept()
    }

    override fun visit(node: ReturnStatement) {
        node.expression.accept()
    }

    override fun visit(node: WhileStatement) {
        node.condition.accept()
        node.statement.accept()
    }

    override fun visit(node: IteratorStatement) {
        when (node.method) {
            is AssignMethod.Decompose -> {
                node.method.entries.map { entry -> entry.key }.accept()
            }

            is AssignMethod.Single -> Unit
        }

        node.iterable.accept()
        node.statement.accept()
    }

    override fun visit(node: YieldStatement) = Unit

    override fun visit(node: AccessAccessible) {
        node.key.accept()
        node.container.accept()
    }

    override fun visit(node: VariableAccessible) = Unit

    private fun Node.accept() = accept(this@SimpleVisitor)
    private fun List<Node>.accept() = forEach { node -> node.accept(this@SimpleVisitor) }
}