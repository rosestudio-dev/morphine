/*
 * Morphine assembler
 */

val println = env.lib("base.println")

fun factorial(n) = asm(emt) {
    section data
        zero 0
        one  1
    section slots
        slot num
    section code
        arg     n, num
        load    zero, emt
        eq      num, emt, emt
        jmpif   emt, ext, cll
ext:    load    one, emt
        ret     emt
cll:    load    one, emt
        sub     num, emt, emt
        param   emt, 0
        invoked emt
        call    emt, 1
        result  emt
        mul     emt, num, emt
}

println(factorial(20))
