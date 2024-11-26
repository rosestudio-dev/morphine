/*
 * 99 bottles of beer
 */

val base = env.library("base")
val tostr = env.library("value.tostr")

fun bottles<tostr>(count) =
    tostr(count) .. " bottle" .. if (count == 1) { "" } else { "s" }

var count = 99
while(count > 0) {
    base.println(bottles(count) .. ' of beer on the wall')
    base.println(bottles(count) .. ' of beer')
    base.println('Take one down, pass it around')
    count --
    base.println(bottles(count) .. ' of beer on the wall')
    base.println()
}
