package ru.unit.morphine.assembly.cli.compiler

import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import kotlinx.cli.default
import kotlinx.cli.required
import ru.unit.morphine.assembly.bytecode.BytecodeConverter
import ru.unit.morphine.assembly.compiler.MorphineAssemble
import java.nio.file.Files
import java.nio.file.Path

fun main(args: Array<String>) {
    val parser = ArgParser(args.getOrNull(0) ?: "morphine-compiler")

    val path by parser.option(
        ArgType.String,
        shortName = "f",
        fullName = "file",
        description = "Program file path"
    ).required()

    val disassembly by parser.option(
        ArgType.Boolean,
        shortName = "d",
        fullName = "disassembly",
        description = "Disassembly program"
    ).default(false)

    val disableOptimizer by parser.option(
        ArgType.Boolean,
        shortName = "p",
        fullName = "disable-optimizer",
        description = "Disable optimizer"
    ).default(false)

    val output by parser.option(
        ArgType.String,
        shortName = "o",
        fullName = "output",
        description = "Output file path"
    ).default("")

    val bytes by parser.option(
        ArgType.Boolean,
        shortName = "b",
        fullName = "bytes",
        description = "Output human format bytes"
    ).default(false)

    val debug by parser.option(
        ArgType.Boolean,
        fullName = "debug",
    ).default(false)

    parser.parse(args)

    compile(
        file = path,
        output = output,
        bytes = bytes,
        optimizer = !disableOptimizer,
        disassembly = disassembly,
        debug = debug
    )
}

fun compile(
    file: String,
    output: String,
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

                if (output.isNotBlank()) {
                    Files.write(Path.of(output), out)
                }

                when {
                    output.isBlank() && !bytes -> System.out.write(out)
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
