val { version, pcall, ecall, println } = env.library("base")
val exception = env.library("exception")
val compiler = env.library("compiler")
val app = env.library("__app")

val fakelib = env.load("utils.fakelib")

fun repl<auto>() {
    val data = { exit = false }

    fun wrapper(text) = "return do { " .. text .. "}"

    fun exec<auto>(text) {
        val compiled = compiler.compile {
            text = wrapper(text),
            name = "replfun"
        }

        return ecall(data.environment, compiled)
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

    fun help<auto>() = "Support functions:
    type 'env.exit()'    to exit
    type 'env.block()'   to execute block
    type 'env.restore()' to restore env
    type 'env.help()'    to print this message"

    fun recursive restore<auto>() {
        data.environment = {
            library = fakelib,
            exit = exit,
            block = block,
            restore = restore,
            help = help,
        }
    }

    restore()

    println("Welcome to morphine REPL!")
    println("Version: " .. version)
    println()
    println(help())

    while (not data.exit) {
        val line = app.readline("> ")

        val result = pcall(exec, line)
        if (typeof result == "exception") {
            exception.print(result)
        } else {
            println(result)
        }
    }

    println("Goodbye!")
    app.exit(0)
}

return { run = repl }
