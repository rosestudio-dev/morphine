package ru.unit.morphine.assembly.compiler.ast.visitor

import ru.unit.morphine.assembly.compiler.ast.node.AccessAccessible
import ru.unit.morphine.assembly.compiler.ast.node.AssigmentStatement
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
import ru.unit.morphine.assembly.compiler.ast.node.VectorExpression
import ru.unit.morphine.assembly.compiler.ast.node.Visitor
import ru.unit.morphine.assembly.compiler.ast.node.WhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.YieldStatement

abstract class AbstractVisitor : Visitor {

    override fun visit(node: Node) = when (node) {
        is BinaryExpression -> visit(node)
        is BlockStatement -> visit(node)
        is CallExpression -> visit(node)
        is CallSelfExpression -> visit(node)
        is EnvExpression -> visit(node)
        is FunctionExpression -> visit(node)
        is IfStatement -> visit(node)
        is IncDecExpression -> visit(node)
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
    }

    abstract fun visit(node: BinaryExpression)
    abstract fun visit(node: BlockStatement)
    abstract fun visit(node: CallExpression)
    abstract fun visit(node: CallSelfExpression)
    abstract fun visit(node: EnvExpression)
    abstract fun visit(node: FunctionExpression)
    abstract fun visit(node: IfStatement)
    abstract fun visit(node: IncDecExpression)
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
}