/*
 * Ast
 */

val code = "
    // Code...
"

val base = env.library("base")
val compiler = env.library("compiler")

base.println(compiler.ast(code))
