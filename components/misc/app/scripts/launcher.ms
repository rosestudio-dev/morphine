val { version, pcall, ecall, println } = env.lib("base")
val { exit } = env.lib("__app")
val { tostr } = env.lib("value")
val { pack, unpack } = env.lib("packer")
val exception = env.lib("exception")
val compiler = env.lib("compiler")
val vector = env.lib("vector")
val sio = env.lib("sio")
val fs = env.lib("fs")
val gc = env.lib("gc")

val parseargs = env.load("utils.parseargs")
val fakelib = env.load("utils.fakelib")
val repl = env.load("repl")

val pattern = {
    program = if (lenof env.args > 0) { env.args[0] } else { "morphine" },
    version,
    optional = [
        { name = "packed",      key = "p", type = parseargs.type.toggle,                                    desc = "Packed program" },
        { name = "inline",      key = "i", type = parseargs.type.toggle,                                    desc = "Inline program" },
        { name = "run",         key = "r", type = parseargs.type.toggle,                                    desc = "Run program" },
        { name = "disassembly", key = "d", type = parseargs.type.toggle,                                    desc = "Disassembly program" },
        { name = "export",      key = "e", type = parseargs.type.string, value = "path",                    desc = "Export program" },
        { name = "name",        key = "n", type = parseargs.type.string, value = "name",  default = "main", desc = "Program name (gives an effect for a non-packed format)" },
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

val content = pcall <- fun<auto>() = if (config.packed) {
    val file = fs.open(config.program, "rb")
    val result = unpack(file)
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
    pack(file, content)
    sio.close(file)
}

if (config.run) {
    val environment = {
        lib = fakelib,
        args = vector.copy(config.args)
    }

    val result = ecall(environment, content)

    if (config.inline) {
        println(result)
    }
}
