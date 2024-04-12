package ru.unit.morphine.assembly.compiler

import ru.unit.morphine.assembly.bytecode.Bytecode
import ru.unit.morphine.assembly.bytecode.LineData
import ru.unit.morphine.assembly.bytecode.Value
import ru.unit.morphine.assembly.compiler.ast.Ast
import ru.unit.morphine.assembly.compiler.lexer.Token

object Printer {

    fun tokens(tokens: List<Pair<Token, LineData>>) {
        println("Tokens:")

        val lines = tokens.map { (_, lineData) ->
            lineData.line
        }.distinct().sorted()

        lines.forEach { line ->
            val tokensInLine = tokens.filter { (_, lineData) ->
                lineData.line == line
            }.map(Pair<Token, LineData>::first)

            println("    $line: ${tokensInLine.joinToString()}")
        }

        println()
    }

    fun bytecode(bytecode: Bytecode, source: String) {
        bytecode.functions.forEach { function ->
            bytecodeFunction(source, bytecode, function)
        }
    }

    private fun bytecodeFunction(source: String, bytecode: Bytecode, function: Bytecode.Function) {
        val lines = source.lines()

        val main = if (bytecode.mainFunction == function.uuid) {
            " main"
        } else {
            ""
        }

        val optimized = if(function.optimize) {
            " optimized"
        } else {
            ""
        }

        println("function '${function.name}'(${function.uuid})$main$optimized (instructions: ${function.instructions.size}, constants: ${function.constants.size}, args: ${function.argumentsCount}, closures: ${function.closuresCount}, statics: ${function.staticsCount}, slots: ${function.slotsCount}, params: ${function.paramsCount}):")

        if (function.instructions.isNotEmpty()) {
            val maxLenLine = function.instructions.maxOf { (it.lineData?.line?.toString() ?: "?").length }
            val maxLenIndex = function.instructions.size.toString().length
            val maxLenName = function.instructions.maxOf { it.opcode.name.length }
            val maxLenArg1 = function.instructions.maxOf { it.orderedArguments.getOrNull(0)?.toString()?.length ?: 0 }
            val maxLenArg2 = function.instructions.maxOf { it.orderedArguments.getOrNull(1)?.toString()?.length ?: 0 }
            val maxLenArg3 = function.instructions.maxOf { it.orderedArguments.getOrNull(2)?.toString()?.length ?: 0 }

            val printedSourceLines = mutableSetOf(0)

            function.instructions.forEachIndexed { index, instruction ->
                val line = instruction.lineData?.line?.toString() ?: "?"
                val i = "$index"
                val name = instruction.opcode.name
                val arg1 = instruction.orderedArguments.getOrNull(0)?.toString() ?: ""
                val arg2 = instruction.orderedArguments.getOrNull(1)?.toString() ?: ""
                val arg3 = instruction.orderedArguments.getOrNull(2)?.toString() ?: ""

                val result = buildString {
                    append("        ")

                    append(line)
                    append(" ".repeat(maxLenLine - line.length))
                    append(" | ")

                    append("[$i]")
                    append(" ".repeat(maxLenIndex - i.length))
                    append("  ")

                    append(name)
                    append(" ".repeat(maxLenName - name.length))
                    append("    ")

                    append(arg1)
                    append(" ".repeat(maxLenArg1 - arg1.length))
                    append("    ")

                    append(arg2)
                    append(" ".repeat(maxLenArg2 - arg2.length))
                    append("    ")

                    append(arg3)
                    append(" ".repeat(maxLenArg3 - arg3.length))
                    append("    ; ")

                    append(instruction.description)
                }

                val lineNum = instruction.lineData?.line ?: 0

                if(lineNum !in printedSourceLines && (lineNum - 1) in lines.indices) {
                    println("    ${lines[lineNum - 1].trim()}")
                }

                println(result)

                printedSourceLines.add(lineNum)
            }
        }

        if (function.constants.isNotEmpty()) {
            println("constants: ")

            val maxLenIndex = function.constants.size.toString().length
            val maxLenName = function.constants.maxOf { type(it).length }
            val maxLenValue = function.constants.maxOf { it.toString().length }

            function.constants.forEachIndexed { index, constant ->
                val i = "$index"
                val name = type(constant)
                val value = constant.toString()

                val result = buildString {
                    append("    ")

                    append("[$i]")
                    append(" ".repeat(maxLenIndex - i.length))
                    append(" ")

                    append(name)
                    append(" ".repeat(maxLenName - name.length))
                    append(" ")

                    append(value)
                    append(" ".repeat(maxLenValue - value.length))
                }

                println(result)
            }
        }

        println()
    }

    private fun type(value: Value) = when (value) {
        is Value.Boolean -> "boolean"
        is Value.Integer -> "integer"
        is Value.Function -> "function"
        Value.Nil -> "nil"
        is Value.Decimal -> "decimal"
        is Value.String -> "string"
    }
}