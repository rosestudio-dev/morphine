val { isalpha, tolower, substring } = env.library("string")
val { tostr, toint } = env.library("value")
val { error } = env.library("base")

return fun tobytes<auto>(value) {
    val last = value[lenof value - 1]

    val param = toint <- if (isalpha(last)) {
        substring(value, 0, lenof value - 1)
    } else {
        value
    }

    val modifier = if (isalpha(last)) {
        tolower(last)
    }

    return when (modifier) {
        nil -> param,
        'k' -> param * 1024,
        'm' -> param * 1024 * 1024,
        'g' -> param * 1024 * 1024 * 1024,
        else -> error("undefined modifier '" .. tostr(modifier) .. "'")
    }
}
