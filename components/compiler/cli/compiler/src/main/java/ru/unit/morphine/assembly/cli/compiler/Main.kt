package ru.unit.morphine.assembly.cli.compiler

import java.nio.file.Files
import java.nio.file.Path
import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import ru.unit.morphine.assembly.bytecode.BytecodeConverter
import ru.unit.morphine.assembly.compiler.MorphineAssemble

fun main(args: Array<String>) {
    val programName = args.getOrNull(0) ?: "compiler"
    val parser = ArgParser(programName)

    val path = parser.option(
        ArgType.String,
        shortName = "f",
        fullName = "file",
        description = "Program file path"
    )

    val disassembly = parser.option(
        ArgType.Boolean,
        shortName = "d",
        fullName = "disassembly",
        description = "Disassembly program"
    )

    val disableOptimizer = parser.option(
        ArgType.Boolean,
        shortName = "p",
        fullName = "disable-optimizer",
        description = "Disable optimizer"
    )

    val output = parser.option(
        ArgType.String,
        shortName = "o",
        fullName = "output",
        description = "Output file path"
    )

    val bytes = parser.option(
        ArgType.Boolean,
        shortName = "b",
        fullName = "bytes",
        description = "Output human format bytes"
    )

    val version = parser.option(
        ArgType.Boolean,
        shortName = "v",
        fullName = "version",
        description = "Print version"
    )

    val debug = parser.option(
        ArgType.Boolean,
        fullName = "debug",
    )

    parser.parse(args)

    var printNTD = true

    if (version.value == true) {
        printNTD = false
        println("Compiler version: ${MorphineAssemble::class.java.`package`.implementationVersion}")
    }

    if (!path.value.isNullOrBlank()) {
        printNTD = false
        compile(
            file = path.value!!,
            output = output.value ?: "",
            bytes = bytes.value ?: false,
            optimizer = (disableOptimizer.value ?: true),
            disassembly = disassembly.value ?: false,
            debug = debug.value ?: false
        )
    }

    if (printNTD) {
        println("Nothing to do.")
        println("Try '$programName --help' for more information.")
    }
}

private fun compile(
    file: String,
    output: String?,
    bytes: Boolean,
    optimizer: Boolean,
    disassembly: Boolean,
    debug: Boolean,
) {
    runCatching {
        val text = Files.readString(Path.of(file))

        val result = MorphineAssemble(
            text = text,
            optimize = optimizer,
            print = disassembly,
            debug = debug
        ).assemble()

        when (result) {
            is MorphineAssemble.Result.Error -> {
                println(result.message)
            }

            is MorphineAssemble.Result.Success -> {
                val out = BytecodeConverter().convert(result.bytecode).toByteArray()

                if (!output.isNullOrBlank()) {
                    Files.write(Path.of(output), out)
                }

                when {
                    output.isNullOrBlank() && !bytes -> System.out.write(out)
                    bytes -> printCompiled(out)
                }
            }
        }
    }.onFailure { throwable ->
        if (debug) {
            throw throwable
        }
    }
}

private fun printCompiled(out: ByteArray) {
    print("Compiled ${out.size} bytes")
    if (out.isEmpty()) {
        println()
    } else {
        print(":")
    }

    repeat(out.size) { index ->
        if (index % 8 == 0) {
            println()
        }

        val hex = out[index].toUByte().toString(16).let { hex ->
            if (hex.length == 1) {
                "0$hex"
            } else {
                hex
            }
        }.uppercase()

        print("0x$hex${if (index == out.size - 1) "" else ", "}")
    }
}
