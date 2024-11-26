//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/compiler.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/visitor.h"
#include "morphinec/codegen.h"

MORPHINE_API void mcapi_compile(
    morphine_coroutine_t U,
    const char *name,
    bool vector
) {
    struct mc_strtable *T = mcapi_push_strtable(U);
    struct mc_ast *A = mcapi_push_ast(U);

    mapi_peek(U, 2);
    struct mc_lex *L = mcapi_push_lex(U);
    struct mc_parser *P = mcapi_push_parser(U);
    while (mcapi_parser_step(U, P, A, L, T)) { }
    mapi_pop(U, 3);

    struct mc_visitor *V = mcapi_push_visitor(U);
    struct mc_codegen *G = mcapi_push_codegen(U);

    while (mcapi_codegen_step(U, G, V, T, A)) { }
    mcapi_codegen_build(U, G, T, A, name, vector);

    mapi_rotate(U, 6);
    mapi_pop(U, 5);
}
