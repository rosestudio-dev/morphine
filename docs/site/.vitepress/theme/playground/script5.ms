/*
 * Tokenize
 */

val code = "
    // Code...
"

val base = env.library("base")
val compiler = env.library("compiler")

val strtable = compiler.strtable.create()
val lex = compiler.lex.create(code)

do
    val token = compiler.lex.token(lex, strtable)
    base.println(token)
end while(token.type != "eos")
