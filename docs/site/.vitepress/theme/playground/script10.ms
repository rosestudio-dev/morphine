/*
 * 1000 Fibonacci numbers
 */

val base = env.lib("base")
val { serialize } = env.lib("value")
val format = env.lib("string.format")
val bigint = env.lib("bigint.wrap")

fun cachable<auto>(function) = base.setmetatable({ function, cache = { } }) {
    fun _mf_call<base, serialize>(s, arguments) {
        val key = serialize(arguments) .. serialize(s)
        val result = self.cache[key] or base.vcall!s(self.function, arguments .. [self])

        self.cache[key] = result
        return result
    }
}

val fib = cachable <- fun<bigint>(n, f) = if(n < 2) {
    bigint(1)
} else {
    f(n - 2) + f(n - 1)
}

for(var i = 1; i <= 1000; i++) {
    base.println(format("${i}. ${num}") { i, num = fib(i) })
}
