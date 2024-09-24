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
val app = env.library("__app")

val originlibrary = env.library
fun fakelibrary<auto>(name, reload)
    val disabled = [
        "__app"
    ]

    iterator(extract value in disabled)
        if (value == name)
            base.error("library not found")
        end
    end

    return if (reload == nil)
        originlibrary(name)
    else
        originlibrary(name, reload)
    end
end

fun errln<auto>(error)
    sio.flush(sio.io())
    sio.write(sio.error(), value.tostr(error) .. "\n")
    sio.flush(sio.error())
end

fun panic<auto>(error)
    errln(error)
    app.exit(1)
end

fun help<auto>(program, message)
    base.println("Morphine (version: " .. base.version().. ")")

    if (message != nil)
        errln("Message: " .. value.tostr(message))
    end

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
end

fun repl<auto>()
    val data = {
        exit = false
    }

    fun exec<auto>(text)
        val compiled = compiler.compile {
            text = text,
            name = "replfun",
            expression = true
        }

        return base.changeenv(data.environment, compiled)
    end

    fun exit<auto>()
        data.exit = true
    end

    fun block<auto>()
        var text = ""
        do
            val line = app.readline(">> ")
            text = text .. line .. "\n"
        end while (line != "")

        return exec(text)
    end

    fun help<auto>() =
"Support functions:
    type 'env.exit()'    to exit
    type 'env.block()'   to execute block
    type 'env.restore()' to restore env
    type 'env.help()'    to print this message"

    fun recursive restore<auto>()
        data.environment ={
            library = fakelibrary,
            exit = exit,
            block = block,
            restore = restore,
            help = help,
        }
    end

    restore()

    base.println("Welcome to morphine REPL!")
    base.println("Version: " .. base.version())
    base.println()
    base.println(help())

    while(not data.exit)
        val line = app.readline("> ")

        val extract result, error = base.pcall(exec, line)
        if (error != nil)
            base.println("error was caused:")
            errln(error)
        else
            base.println(result)
        end
    end

    base.println("Goodbye!")
end

fun tobytes<auto>(program, param)
    if (lenof param < 1)
        help(program, "invalid bytes param")
    end

    fun convert<auto>()
        val extract modifier, param = do
            val last = param[lenof param - 1]

            val int = if (string.isalpha(last))
                value.toint(string.substring(param, 0, lenof param - 1))
            else
                value.toint(param)
            end

            val modifier = if (string.isalpha(last))
                string.tolower(last)
            else
                nil
            end

            { modifier = modifier, param = int }
        end

        return if (modifier == nil)
            param
        elif (modifier == 'k')
            param * 1024
        elif (modifier == 'm')
            param * 1024 * 1024
        elif (modifier == 'g')
            param * 1024 * 1024 * 1024
        else
            nil
        end
    end

    val extract result, error = base.pcall(convert)

    if (result == nil)
        help(program, "invalid bytes param")
    end

    return result
end

fun parse_arguments<auto>()
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

    if (lenof env.args < 1)
        help("morphine", "empty command line arguments")
    end

    if (lenof env.args == 1)
        result.repl = true
        return result
    end

    var require_path = false
    var parsed = 1
    for(pass; parsed < lenof env.args; parsed ++)
        val argument = env.args[parsed]
        if (lenof argument == 0)
            help(result.app_path, "empty command line arguments")
        end

        if (argument[0] != '-')
            break
        end

        var offset = 0
        var stop = false
        for (var i = 1; i < lenof argument; i ++)
            val char = argument[i]
            if (char == '-')
                stop = true
                break
            elif (char == 'b')
                require_path = true
                result.binary = true
            elif (char == 'r')
                require_path = true
                result.execute = true
            elif (char == 'd')
                require_path = true
                result.disassembly = true
            elif (char == 'e')
                require_path = true
                offset ++
                if (parsed + offset >= lenof env.args)
                    help(result.app_path, "no export path")
                end
                result.export = env.args[parsed + offset]
            elif (char == 'n')
                require_path = true
                offset ++
                if (parsed + offset >= lenof env.args)
                    help(result.app_path, "no name")
                end
                result.name = env.args[parsed + offset]
            elif (char == 'm')
                offset ++
                if (parsed + offset >= lenof env.args)
                    help(result.app_path, "no bytes value")
                end
                val value = env.args[parsed + offset]
                result.gc_limit = tobytes(result.app_path, value)
            elif (char == 'h')
                result.help = true
            else
                help(result.app_path, "undefined argument '" .. char .. "'")
            end
        end

        parsed += offset

        if (stop)
            break
        end
    end

    if (parsed >= lenof env.args)
        if (require_path)
            help(result.app_path, "program path is expected")
        else
            return result
        end
    end

    result.program_path = env.args[parsed]
    result.args = parsed + 1

    return result
end

fun launch<auto>()
    val extract config as "result" = base.pcall(parse_arguments)

    if (config == nil)
        leave
    end

    if (config.repl)
        repl()
        leave
    end

    if (config.help)
        help(config.app_path)
    end

    gc.changelimit(config.gc_limit)

    if (config.program_path != nil)
        val content = if (config.binary)
            val file = fs.file(config.program_path, "rb")
            val result = binary.receive(file)
            sio.close(file)

            result
        else
            val file = fs.file(config.program_path, "rb")
            val text = sio.readall(file)
            sio.close(file)

            fun compile<auto>() = compiler.compile {
                text = text,
                name = config.name
            }

            val extract result, error = base.pcall(compile)
            if (error != nil)
                panic(error)
            end

            result
        end

        if (config.disassembly)
            compiler.disassembly(content)
            base.println()
        end

        if (config.export != nil)
            val file = fs.file(config.export, "wb")
            binary.send(file, content)
            sio.close(file)
        end

        if (config.execute)
            val environment = {
                library = fakelibrary,
                args = vector.unfixed([])
            }

            for(var i = config.args; i < lenof env.args; i ++)
                vector.push(environment.args, env.args[i])
            end

            base.changeenv(environment, content)
        end
    end
end

launch()
