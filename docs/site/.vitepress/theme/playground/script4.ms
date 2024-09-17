/*
 * PI number generator
 */

val base = env.library("base")
val bigint = env.library("bigint.wrap")

fun calcPi<bigint>(print)
    val ZERO = bigint(0)
    val ONE = bigint(1)
    val TWO = bigint(2)
    val THREE = bigint(3)
    val FOUR = bigint(4)
    val SEVEN = bigint(7)
    val TEN = bigint(10)

    var q = ONE
    var r = ZERO
    var t = ONE
    var k = ONE
    var n = THREE
    var l = THREE

    while(true)
        if(FOUR * q + r - t < n * t)
            print(n)
            val nr = TEN * (r - n * t)
            n = ((TEN * (THREE * q + r)) / t) - TEN * n
            q *= TEN
            r = nr
        else
            val nr = (TWO * q + r) * l
            val nn = (q * (SEVEN * k) + TWO + (r * l)) / (t * l)
            q *= k
            t *= l
            l += TWO
            k += ONE
            n = nn
            r = nr
        end
    end
end

fun print<base>(text)
    static(index)
    if (index == nil)
        index = 0
    end

    base.print(text)

    index ++
    if (index == 40)
        base.println()
        index = 0
    end
end

calcPi(print)
