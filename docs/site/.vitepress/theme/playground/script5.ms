/*
 * Tokenize
 */

val code = "
    // Code...
"

val base = env.lib("base")
val serialize = env.lib("value.serialize")
val compiler = env.lib("compiler")

val tokens = compiler.lex(code)

iterator({ value } in tokens) {
    base.println(serialize(value))
}
