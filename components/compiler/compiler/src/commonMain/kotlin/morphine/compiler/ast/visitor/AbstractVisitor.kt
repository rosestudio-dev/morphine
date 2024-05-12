package morphine.compiler.ast.visitor

import morphine.compiler.ast.node.AccessAccessible
import morphine.compiler.ast.node.AssigmentStatement
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
import morphine.compiler.ast.node.Visitor
import morphine.compiler.ast.node.WhileStatement
import morphine.compiler.ast.node.YieldStatement

abstract class AbstractVisitor : Visitor {

    override fun visit(node: Node) = when (node) {
        is BinaryExpression -> visit(node)
        is BlockStatement -> visit(node)
        is CallExpression -> visit(node)
        is CallSelfExpression -> visit(node)
        is EnvExpression -> visit(node)
        is FunctionExpression -> visit(node)
        is IfStatement -> visit(node)
        is IncrementExpression -> visit(node)
        is SelfExpression -> visit(node)
        is TableExpression -> visit(node)
        is UnaryExpression -> visit(node)
        is ValueExpression -> visit(node)
        is AssigmentStatement -> visit(node)
        is BreakStatement -> visit(node)
        is ContinueStatement -> visit(node)
        is DeclarationStatement -> visit(node)
        is DoWhileStatement -> visit(node)
        is EmptyStatement -> visit(node)
        is EvalStatement -> visit(node)
        is ForStatement -> visit(node)
        is ReturnStatement -> visit(node)
        is WhileStatement -> visit(node)
        is YieldStatement -> visit(node)
        is IteratorStatement -> visit(node)
        is AccessAccessible -> visit(node)
        is VariableAccessible -> visit(node)
        is VectorExpression -> visit(node)
        is BlockExpression -> visit(node)
        is IfExpression -> visit(node)
    }

    abstract fun visit(node: BinaryExpression)
    abstract fun visit(node: BlockStatement)
    abstract fun visit(node: CallExpression)
    abstract fun visit(node: CallSelfExpression)
    abstract fun visit(node: EnvExpression)
    abstract fun visit(node: FunctionExpression)
    abstract fun visit(node: IfStatement)
    abstract fun visit(node: IncrementExpression)
    abstract fun visit(node: SelfExpression)
    abstract fun visit(node: TableExpression)
    abstract fun visit(node: UnaryExpression)
    abstract fun visit(node: ValueExpression)
    abstract fun visit(node: AssigmentStatement)
    abstract fun visit(node: BreakStatement)
    abstract fun visit(node: ContinueStatement)
    abstract fun visit(node: DeclarationStatement)
    abstract fun visit(node: DoWhileStatement)
    abstract fun visit(node: EmptyStatement)
    abstract fun visit(node: EvalStatement)
    abstract fun visit(node: ForStatement)
    abstract fun visit(node: ReturnStatement)
    abstract fun visit(node: WhileStatement)
    abstract fun visit(node: YieldStatement)
    abstract fun visit(node: IteratorStatement)
    abstract fun visit(node: AccessAccessible)
    abstract fun visit(node: VariableAccessible)
    abstract fun visit(node: VectorExpression)
    abstract fun visit(node: BlockExpression)
    abstract fun visit(node: IfExpression)
}