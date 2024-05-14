package morphine.compiler.ast.visitor

import morphine.compiler.ast.node.AccessAccessible
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
import morphine.compiler.ast.node.ForStatement
import morphine.compiler.ast.node.FunctionExpression
import morphine.compiler.ast.node.IfExpression
import morphine.compiler.ast.node.IfStatement
import morphine.compiler.ast.node.IncrementExpression
import morphine.compiler.ast.node.IteratorStatement
import morphine.compiler.ast.node.Node
import morphine.compiler.ast.node.ReturnStatement
import morphine.compiler.ast.node.SelfExpression
import morphine.compiler.ast.node.TableExpression
import morphine.compiler.ast.node.UnaryExpression
import morphine.compiler.ast.node.ValueExpression
import morphine.compiler.ast.node.VariableAccessible
import morphine.compiler.ast.node.VectorExpression
import morphine.compiler.ast.node.WhileStatement
import morphine.compiler.ast.node.YieldStatement

abstract class SimpleVisitor : AbstractVisitor() {

    override fun visit(node: BinaryExpression) {
        node.expressionA.accept()
        node.expressionB.accept()
    }

    override fun visit(node: BlockStatement) {
        node.statements.accept()
    }

    override fun visit(node: BlockExpression) {
        node.statements.accept()
        node.expression.accept()
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

    override fun visit(node: IfExpression) {
        node.condition.accept()
        node.ifExpression.accept()
        node.elseExpression.accept()
    }

    override fun visit(node: IncrementExpression) {
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

    override fun visit(node: VectorExpression) {
        node.elements.accept()
    }

    private fun Node.accept() = accept(this@SimpleVisitor)
    private fun List<Node>.accept() = forEach { node -> node.accept(this@SimpleVisitor) }
}