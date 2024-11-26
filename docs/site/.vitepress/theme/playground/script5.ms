/*
 * Tokenize
 */

val code = "
    // Code...
"

val base = env.library("base")
val serialize = env.library("value.serialize")
val compiler = env.library("compiler")

val tokens = compiler.lex(code)

iterator({ value } in tokens) {
    base.println(serialize(value))
}
