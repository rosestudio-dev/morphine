/*
 * Ast
 */

val code = "
    // Code...
"

val base = env.library("base")
val serialize = env.library("value.serialize")
val compiler = env.library("compiler")

base.println(serialize(compiler.ast(code)) { pretty = true, tab = 2 })
