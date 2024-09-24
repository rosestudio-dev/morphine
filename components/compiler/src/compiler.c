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
    bool expression,
    bool vector
) {
    const char *text = mapi_get_string(U);
    ml_size text_len = mapi_string_len(U);

    struct mc_strtable *T = mcapi_push_strtable(U);
    struct mc_ast *A = mcapi_push_ast(U);

    {
        struct mc_lex *L = mcapi_push_lex(U, text, text_len);
        struct mc_parser *P = mcapi_push_parser(U, expression);
        while (mcapi_parser_step(U, P, A, L, T)) { }
        mapi_pop(U, 2);
    }

    {
        struct mc_visitor *V = mcapi_push_visitor(U);
        struct mc_codegen *G = mcapi_push_codegen(U);

        while (mcapi_codegen_step(U, G, V, T, A)) { }
        mcapi_codegen_build(U, G, T, A, name, vector);
    }

    mapi_rotate(U, 6);
    mapi_pop(U, 5);
}
