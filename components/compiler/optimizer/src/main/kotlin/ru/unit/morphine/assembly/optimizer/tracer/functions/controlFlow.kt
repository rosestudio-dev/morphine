package ru.unit.morphine.assembly.optimizer.tracer.functions

import org.jgrapht.graph.DefaultEdge
import org.jgrapht.nio.DefaultAttribute
import org.jgrapht.nio.dot.DOTExporter
import ru.unit.morphine.assembly.optimizer.tracer.ControlFlowTree
import ru.unit.morphine.assembly.optimizer.tracer.Tracer
import java.io.StringWriter
import java.nio.file.Files
import java.nio.file.Path


fun Tracer.Data.controlFlow(debug: Boolean): ControlFlowTree {
    val tree = ControlFlowTree(this)
    controlFlowTree = tree

//    if (debug) {
//        val exporter = DOTExporter<Tracer.Block, DefaultEdge> { block ->
//            "block${block.id}"
//        }
//
//        exporter.setVertexAttributeProvider { block ->
//            val label = "block${block.id}"
//            mapOf("label" to DefaultAttribute.createAttribute(label))
//        }
//
//        StringWriter().apply {
//            exporter.exportGraph(tree.graph, this)
//            Files.writeString(Path.of("flow.dot"), toString())
//        }
//
//        StringWriter().apply {
//            exporter.exportGraph(tree.dominatorTree.iDomGraph, this)
//            Files.writeString(Path.of("idom.dot"), toString())
//        }
//    }

    return tree
}