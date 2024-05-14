package morphine.cli

import compiler.cli.BuildConfig
import kotlin.system.exitProcess
import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import kotlinx.io.buffered
import kotlinx.io.files.Path
import kotlinx.io.files.SystemFileSystem
import kotlinx.io.readString
import morphine.Compiler
import morphine.compiler.Printer

fun cli(args: Array<String>) {
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
    optimizer: Boolean,
    disassembly: Boolean,
    debug: Boolean,
) {
    val text = runCatching {
        SystemFileSystem.source(Path(file))
            .buffered().use { source -> source.readString() }
    }.getOrElse { throwable ->
        println(throwable.message)
        exitProcess(1)
    }

    val result = Compiler(
        text = text,
        optimize = optimizer,
    ).compileBytes()

    when (result) {
        is Compiler.Result.Error -> result.parseResult(
            debug = debug
        )

        is Compiler.Result.Success -> result.parseResult(
            output = output,
            text = text,
            disassembly = disassembly
        )
    }
}

private fun Compiler.Result.Success<List<Byte>>.parseResult(
    output: String?,
    text: String,
    disassembly: Boolean,
) {
    if (disassembly) {
        Printer.bytecode(bytecode, text)
    }

    val out = data.toByteArray()

    if (output.isNullOrBlank()) {
        printCompiled(out)
    } else {
        runCatching {
            SystemFileSystem.sink(Path(output)).buffered().use { sink ->
                sink.write(out)
            }
        }.onFailure { throwable ->
            println(throwable.message)
            exitProcess(1)
        }
    }
}

private fun Compiler.Result.Error<List<Byte>>.parseResult(
    debug: Boolean,
) {
    if (debug) {
        throw throwable
    } else {
        println(message)
        exitProcess(1)
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
