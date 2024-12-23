val originlib = env.lib
val { error } = originlib("base")
val { trim, startswith } = originlib("string")

val disabled = env.private_libs

return fun lib<auto>(name) {
    val name = trim(name)
    iterator({ value } in disabled) {
        if (name == value or startswith(name, value .. ".")) {
            error("library not found")
        }
    }

    return originlib(name)
}