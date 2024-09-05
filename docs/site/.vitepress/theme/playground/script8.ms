/*
 * Colored string
 */

val string = "hello_world!"

val base = env.library("base")
val value = env.library("value")

fun printchar<base, value>(char, index)
    val open = "\u1b[3" .. value.tostr(index % 8) .. "m"
    val close = "\u1b[0m"
    base.print(open .. char .. close)
end

iterator(extract key, value in string)
    printchar(value, key)
end
base.println()
