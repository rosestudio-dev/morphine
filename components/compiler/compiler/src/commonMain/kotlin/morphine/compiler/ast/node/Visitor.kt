package morphine.compiler.ast.node

interface Visitor {

    fun visit(node: Node)
}