/*
 * Tokenize
 */

val code = "
    // Code...
"

val base = env.library("base")
val compiler = env.library("compiler")

val tokens = compiler.lex(code)

iterator(extract key, value in tokens)
    base.println(value)
end
