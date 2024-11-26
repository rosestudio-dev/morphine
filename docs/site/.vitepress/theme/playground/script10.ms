/*
 * 1000 Fibonacci numbers
 */

val base = env.library("base")
val tostr = env.library("value.tostr")
val format = env.library("string.format")
val bigint = env.library("bigint.wrap")

fun cachable<auto>(function) = base.setmetatable({ function, cache = { } }) {
    fun _mf_call<base, tostr>(s, arguments) {
        val key = tostr(arguments) .. tostr(s)
        val result = self.cache[key] or base.vcall!s(self.function, arguments .. [self])

        self.cache[key] = result
        return result
    }
}

val cachefib = cachable <- fun<bigint>(n, f) = if(n < 2) {
    bigint(1)
} else {
    f(n - 2) + f(n - 1)
}

for(var i = 1; i <= 1000; i++) {
    base.println(format("${i}. ${num}") { i, num = cachefib(i) })
}
