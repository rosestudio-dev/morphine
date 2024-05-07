package ru.unit.morphine.assembly.cli.compiler

import java.nio.file.Files
import java.nio.file.Path
import kotlin.system.exitProcess
import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import ru.unit.morphine.assembly.cli.compiler.compiler.BuildConfig
import ru.unit.morphine.assembly.compiler.MorphineCompiler
import ru.unit.morphine.assembly.compiler.Printer

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
        println("Compiler version: ${BuildConfig.version}")
    }

    if (!path.value.isNullOrBlank()) {
        printNTD = false
        compile(
            file = path.value!!,
            output = output.value ?: "",
            bytes = bytes.value ?: false,
            optimizer = !(disableOptimizer.value ?: false),
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
    val text = runCatching {
        Files.readString(Path.of(file))
    }.getOrElse {
        println("Cannot open file $file")
        exitProcess(1)
    }

    val result = MorphineCompiler(
        text = text,
        optimize = optimizer,
    ).compile()

    when (result) {
        is MorphineCompiler.Result.Error -> if (debug) {
            throw result.throwable
        } else {
            println(result.message)
            exitProcess(1)
        }

        is MorphineCompiler.Result.Success -> {
            if (disassembly) {
                Printer.bytecode(result.bytecode, text)
            }

            val out = result.data.toByteArray()

            if (!output.isNullOrBlank()) {
                Files.write(Path.of(output), out)
            }

            when {
                output.isNullOrBlank() && !bytes -> System.out.write(out)
                bytes -> printCompiled(out)
            }
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
