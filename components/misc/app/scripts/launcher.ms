val base = env.library("base")
val table = env.library("table")
val vector = env.library("vector")
val string = env.library("string")
val value = env.library("value")
val sio = env.library("sio")
val gc = env.library("gc")
val fs = env.library("fs")
val compiler = env.library("compiler")
val binary = env.library("binary")
val exception = env.library("exception")
val app = env.library("__app")

val originlibrary = env.library
fun fakelibrary<auto>(name) {
    val disabled = ["__app"]

    iterator({ value } in disabled) {
        if (value == name) {
            base.error("library not found")
        }
    }

    return originlibrary(name)
}

fun errln<auto>(error) {
    sio.flush(sio.io())
    if (typeof error == "exception") {
        exception.error.print(error, sio.error())
    } else {
        sio.write(sio.error(), value.tostr(error) .. "\n")
        sio.flush(sio.error())
    }
}

fun panic<auto>(error) {
    errln(error)
    app.exit(1)
}

fun help<auto>(program, message) {
    base.println("Morphine (version: " .. base.version().. ")")

    if (message != nil) {
        errln("Message: " .. value.tostr(message))
    }

    base.println("Usage: " .. program .. " [options] [program [args]]")
    base.println("Positional arguments:")
    base.println("    program         Path to program")
    base.println("    args            Program arguments")
    base.println("Optional arguments:")
    base.println("    -b              Binary program")
    base.println("    -r              Run program")
    base.println("    -e path         Export program")
    base.println("    -n name  [main] Program name (gives an effect for a non-binary format)")
    base.println("    -d              Disassembly program")
    base.println("    -m bytes [256M] Allocation limit (possible suffixes: K, M, G)")
    base.println("    -h              Print help message")
    base.println("    --              Stop handling options")

    app.exit(1)
}

fun repl<auto>() {
    val data = { exit = false }

    fun wrapper(text) = "return do { " .. text .. "}"

    fun exec<auto>(text) {
        val compiled = compiler.compile {
            text = wrapper(text),
            name = "replfun"
        }

        return base.ecall(data.environment, compiled)
    }

    fun exit<auto>() {
        data.exit = true
    }

    fun block<auto>() {
        var text = ""
        do {
            val line = app.readline(">> ")
            text ..= line .. "\n"
        } while (line != "")

        return exec(text)
    }

    fun help<auto>() =
"Support functions:
    type 'env.exit()'    to exit
    type 'env.block()'   to execute block
    type 'env.restore()' to restore env
    type 'env.help()'    to print this message"

    fun recursive restore<auto>() {
        data.environment = {
            library = fakelibrary,
            exit = exit,
            block = block,
            restore = restore,
            help = help,
        }
    }

    restore()

    base.println("Welcome to morphine REPL!")
    base.println("Version: " .. base.version())
    base.println()
    base.println(help())

    while (not data.exit) {
        val line = app.readline("> ")

        val result = base.pcall(exec, line)
        if (typeof result == "exception") {
            exception.print(result)
        } else {
            base.println(result)
        }
    }

    base.println("Goodbye!")
}

fun tobytes<auto>(program, param) {
    if (lenof param < 1) {
        help(program, "invalid bytes param")
    }

    fun convert<auto>() {
        val { modifier, param } = do {
            val last = param[lenof param - 1]

            val int = if (string.isalpha(last)) {
                value.toint(string.substring(param, 0, lenof param - 1))
            } else {
                value.toint(param)
            }

            val modifier = if (string.isalpha(last)) {
                string.tolower(last)
            }

            { modifier = modifier, param = int }
        }

        return when (modifier) {
            nil -> param,
            'k' -> param * 1024,
            'm' -> param * 1024 * 1024,
            'g' -> param * 1024 * 1024
        }
    }

    val result = base.pcall(convert)
    if (not result) {
        help(program, "invalid bytes param")
    }

    return result
}

fun parse_arguments<auto>() {
    val result = table.fixed {
        app_path = env.args[0],
        program_path = nil,
        args = 0,
        gc_limit = 256 * 1024 * 1024,
        binary = false,
        execute = false,
        name = "main",
        disassembly = false,
        export = nil,
        repl = false,
        help = false
    }

    if (lenof env.args < 1) {
        help("morphine", "empty command line arguments")
    }

    if (lenof env.args == 1) {
        result.repl = true
        return result
    }

    var require_path = false
    var parsed = 1

    for(pass; parsed < lenof env.args; parsed ++) {
        val argument = env.args[parsed]
        if (lenof argument == 0) {
            help(result.app_path, "empty command line arguments")
        }

        if (argument[0] != '-') {
            break
        }

        var offset = 0
        var stop = false
        for (var i = 1; i < lenof argument; i ++) {
            val char = argument[i]
            when(char) {
                '-' -> do {
                    stop = true
                    break
                },

                'b' -> do {
                    require_path = true
                    result.binary = true
                },

                'r' -> do {
                    require_path = true
                    result.execute = true
                },

                'd' -> do {
                    require_path = true
                    result.disassembly = true
                },

                'e' -> do {
                    require_path = true
                    offset ++
                    if (parsed + offset >= lenof env.args) {
                        help(result.app_path, "no export path")
                    }
                    result.export = env.args[parsed + offset]
                },

                'n' -> do {
                    require_path = true
                    offset ++
                    if (parsed + offset >= lenof env.args) {
                        help(result.app_path, "no name")
                    }
                    result.name = env.args[parsed + offset]
                },

                'm' -> do {
                    offset ++
                    if (parsed + offset >= lenof env.args) {
                        help(result.app_path, "no bytes value")
                    }
                    val value = env.args[parsed + offset]
                    result.gc_limit = tobytes(result.app_path, value)
                },

                'h' -> do {
                    result.help = true
                },

                else -> help(result.app_path, "undefined argument '" .. char .. "'")
            }
        }

        parsed += offset

        if (stop) {
            break
        }
    }

    if (parsed >= lenof env.args) {
        if (require_path) {
            help(result.app_path, "program path is expected")
        } else {
            return result
        }
    }

    result.program_path = env.args[parsed]
    result.args = parsed + 1

    return result
}

fun launch<auto>() {
    val config = parse_arguments()

    if (config.repl) {
        repl()
        leave
    }

    if (config.help) {
        help(config.app_path)
    }

    gc.settings.setlimit(config.gc_limit)

    if (config.program_path != nil) {
        val content = base.pcall <- fun<auto>() = if (config.binary) {
            val file = fs.open(config.program_path, "rb")
            val result = binary.receive(file)
            sio.close(file)

            result
        } else {
            val file = fs.open(config.program_path, "rb")
            val text = sio.readall(file)
            sio.close(file)

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
            base.println()
        }

        if (config.export != nil) {
            val file = fs.open(config.export, "wb")
            binary.send(file, content)
            sio.close(file)
        }

        if (config.execute) {
            val environment = {
                library = fakelibrary,
                args = vector.unfixed()
            }

            for (var i = config.args; i < lenof env.args; i ++) {
                vector.push(environment.args, env.args[i])
            }

            base.ecall(environment, content)
        }
    }
}

launch()
