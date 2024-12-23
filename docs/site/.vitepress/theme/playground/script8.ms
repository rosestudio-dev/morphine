/*
 * Colored string
 */

val string = "hello_world!"

val { print, println } = env.lib("base")
val { tostr } = env.lib("value")

fun printchar<print, tostr>(char, index) {
    val open = "\u1b[3" .. tostr(index % 8) .. "m"
    val close = "\u1b[0m"
    print(open .. char .. close)
}

iterator({ key, value } in string) {
    printchar(value, key)
}
println()
