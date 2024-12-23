val { println, error } = env.lib("base")
val { format, repeat } = env.lib("string")
val { tostr, tobool, toint } = env.lib("value")
val { fixed } = env.lib("table")
val { trim } = env.lib("vector")
val { exit } = env.lib("__app")
val tobytes = env.load("utils.tobytes")

val type = {
    toggle = "toggle",
    bytes = "bytes",
    string = "string",
    integer = "integer"
}

fun help<auto>(pattern) {
    fun max(a, b) = if (a > b) { a } else { b }
    fun space<repeat>(len) = repeat(" ", len)

    fun type_suffix<type>(value) = when(value) {
        type.toggle -> "",
        type.bytes -> " (possible suffixes: K, M, G)",
        type.string -> "",
        type.integer -> "",
        nil -> ""
    }

    val optional = if (lenof pattern.optional > 0) {
        " [options]"
    } else {
        ""
    }

    val positional = if (pattern.vararg or lenof pattern.positional > 0) {
        var text = " ["
        iterator({ key, value } in pattern.positional) {
            text ..= value.name
            if (key < lenof pattern.positional - 1) {
                text ..= " "
            }
        }

        if (pattern.vararg) {
            if (lenof pattern.positional > 0) {
                text ..= " [" .. pattern.vararg.name .. "]"
            } else {
                text ..= pattern.vararg.name
            }
        }

        text .. "]"
    } else {
        ""
    }

    val positional_args = pattern.positional .. if (pattern.vararg) {
        [pattern.vararg]
    } else {
        []
    }

    val common_space = do {
        var positional_space = 0
        iterator({ value } in positional_args) {
            positional_space = max(positional_space, lenof value.name)
        }

        var optional_space = 0
        iterator({ value } in pattern.optional) {
            val name_len = if (value.value) { (lenof value.value) + 1 } else { 0 }
            val default_len = if (value.default) { (lenof value.default) + 3 } else { 0 }
            optional_space = max(optional_space, 2 + name_len + default_len)
        }

        max(positional_space, optional_space)
    }

    println <- format("Morphine (version: ${version})") { version = pattern.version }
    println <- format("Usage: ${program}${optional}${positional}") { program = pattern.program, optional, positional }

    if (lenof positional_args > 0) {
        println("Positional arguments:")
        iterator({ value } in positional_args) {
            println <- format("    ${name}${tab}${desc}${suffix}") {
                name = value.name,
                tab = space(common_space - lenof value.name),
                desc = if(value.desc) { " " .. value.desc } else { "" },
                suffix = type_suffix(value.type)
            }
        }
    }

    if (lenof pattern.optional > 0) {
        println("Optional arguments:")
        iterator({ value } in pattern.optional) {
            var result = "-" .. value.key[0]

            if (value.value) {
                result ..= " " .. value.value
            }

            if (value.default) {
                val ext = " [" .. tostr(value.default) .. "]"
                result ..= space(common_space - (lenof result + lenof ext)) .. ext
            } else {
                result ..= space(common_space - lenof result)
            }

            if (value.desc) {
                result ..= " " .. value.desc
            }

            result ..= type_suffix(value.type)

            println("    " .. result)
        }
        println <- format("    -h${tab} Print help message") { tab = space(common_space - 2) }
        println <- format("    --${tab} Stop handling options") { tab = space(common_space - 2) }
    }
}

fun get<auto>(args, pattern) {
    val result = do {
        val data = { }

        iterator({ value } in pattern.optional) {
            data[value.name] = when(value.type) {
                type.toggle -> if (value.default) { tobool(value.default) } else { false },
                type.bytes -> value.default and tobytes(value.default),
                type.string -> value.default and tostr(value.default),
                type.integer -> value.default and toint(value.default),
            }
        }

        iterator({ value } in pattern.positional) {
            data[value.name] = nil
        }

        if (pattern.vararg) {
            data[pattern.vararg.name] = []
        }

        data[pattern.noargs] = false

        fixed(data)
    }

    if (lenof args <= 1) {
        result[pattern.noargs] = true
        return result
    }

    var parsed = 1
    for(pass; parsed < lenof args; parsed ++) {
        val argument = args[parsed]
        if (lenof argument == 0) {
            error("empty command line argument")
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
                'h' -> do {
                    help(pattern)
                    exit(0)
                }
            }

            var value = nil
            iterator(entry in pattern.optional) {
                if (entry.value.key[0] == char) {
                    value = entry.value
                }
            }

            if (value == nil) {
                error("undefined argument '" .. char .. "'")
            }

            if (value.type == type.toggle) {
                result[value.name] = true
            } else {
                val function = when(value.type) {
                    type.bytes -> tobytes,
                    type.string -> tostr,
                    type.integer -> toint,
                }

                offset ++
                if (parsed + offset >= lenof args) {
                    error("no value for '" .. value.name .. "' argument")
                }
                result[value.name] = function(args[parsed + offset])
            }
        }

        parsed += offset

        if (stop) {
            break
        }
    }

    if (parsed >= lenof args) {
        error("positional arguments are expected")
    }


    iterator({ value } in pattern.positional) {
        val function = when(value.type) {
            type.bytes -> tobytes,
            type.string -> tostr,
            type.integer -> toint,
        }

        if (parsed >= lenof args) {
            error("no value for '" .. value.name .. "' argument")
        }
        result[value.name] = function(args[parsed])
        parsed ++
    }

    if (pattern.vararg) {
        result[pattern.vararg.name] = trim(args, parsed, lenof args)
    }

    return result
}

return {
    help,
    get,
    type
}
