/*
 * Morphine assembler
 */

val println = env.library("base.println")

fun factorial(n) = asm(ret)
    section data
        zero 0
        one  1
    section slots
        slot num
    section code
        arg     n, num
        load    zero, ret
        eq      num, ret, ret
        jmpif   ret, ext, cll
ext:    load    one, ret
        leave   ret
cll:    load    one, ret
        sub     num, ret, ret
        param   ret, 0
        invoked ret
        call    ret, 1
        result  ret
        mul     ret, num, ret
end

println(factorial(20))
