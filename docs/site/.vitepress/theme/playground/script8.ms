/*
 * Colored string
 */

val string = "hello_world!"

val extract print, println = env.library("base")
val value = env.library("value")

fun printchar<print, value>(char, index)
    val open = "\u1b[3" .. value.tostr(index % 8) .. "m"
    val close = "\u1b[0m"
    print(open .. char .. close)
end

iterator(extract key, value in string)
    printchar(value, key)
end
println()
