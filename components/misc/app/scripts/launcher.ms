val { version, pcall, ecall, println } = env.library("base")
val { exit } = env.library("__app")
val { tostr } = env.library("value")
val exception = env.library("exception")
val compiler = env.library("compiler")
val vector = env.library("vector")
val binary = env.library("binary")
val sio = env.library("sio")
val fs = env.library("fs")
val gc = env.library("gc")

val parseargs = env.load("utils.parseargs")
val fakelib = env.load("utils.fakelib")
val repl = env.load("repl")

val pattern = {
    program = if (lenof env.args > 0) { env.args[0] } else { "morphine" },
    version,
    optional = [
        { name = "binary",      key = "b", type = parseargs.type.toggle,                                    desc = "Binary program" },
        { name = "inline",      key = "i", type = parseargs.type.toggle,                                    desc = "Inline program" },
        { name = "run",         key = "r", type = parseargs.type.toggle,                                    desc = "Run program" },
        { name = "disassembly", key = "d", type = parseargs.type.toggle,                                    desc = "Disassembly program" },
        { name = "export",      key = "e", type = parseargs.type.string, value = "path",                    desc = "Export program" },
        { name = "name",        key = "n", type = parseargs.type.string, value = "name",  default = "main", desc = "Program name (gives an effect for a non-binary format)" },
        { name = "memory",      key = "m", type = parseargs.type.bytes,  value = "bytes", default = "256M", desc = "Allocation limit" },
    ],
    positional = [
        { name = "program", type = parseargs.type.string, desc = "Program path or text (see optional args)" },
    ],
    vararg = { name = "args", desc = "Program arguments" },
    noargs = "repl"
}

fun errln<auto>(error) {
    sio.flush(sio.stream.io)
    if (typeof error == "exception") {
        exception.error.print(error, sio.stream.err)
    } else {
        sio.write(sio.stream.err, tostr(error) .. "\n")
        sio.flush(sio.stream.err)
    }
}

fun panic<auto>(error) {
    errln(error)
    exit(1)
}

val config = pcall(parseargs.get, env.args, pattern)
if (not config) {
    errln(config)
    parseargs.help(pattern)
    exit(1)
}

// runtime

if (config.repl) {
    repl.run()
}

gc.settings.setlimit(config.memory)

val content = pcall <- fun<auto>() = if (config.binary) {
    val file = fs.open(config.program, "rb")
    val result = binary.receive(file)
    sio.close(file)

    result
} else {
    val text = if (config.inline) {
        "return do { " .. config.program .. " }"
    } else {
        val file = fs.open(config.program, "rb")
        val text = sio.readall(file)
        sio.close(file)

        text
    }

    compiler.compile {
        text = text,
        name = config.name
    }
}

if (typeof content == "exception") {
    panic(content)
}

if (config.disassembly) {
    compiler.dis(content)
    println()
}

if (config.export != nil) {
    val file = fs.open(config.export, "wb")
    binary.send(file, content)
    sio.close(file)
}

if (config.run) {
    val environment = {
        library = fakelib,
        args = vector.copy(config.args)
    }

    val result = ecall(environment, content)

    if (config.inline) {
        println(result)
    }
}
