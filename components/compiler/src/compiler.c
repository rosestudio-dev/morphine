//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/compiler.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/codegen.h"
#include "morphinec/decompiler.h"

MORPHINE_API void mcapi_compile(morphine_coroutine_t U, const char *str, size_t size) {
    struct strtable *T = strtable(U);
    struct ast *A = ast(U);

    struct lex *L = lex(U, T, str, size);
    struct parser *P = parser(U, L, A);
    while (parser_step(U, P)) { }
    mapi_pop(U, 2);

    struct visitor *V = visitor(U, A);
    struct codegen *C = codegen(U, T, A, V);
    while (codegen_step(U, C)) { }
    codegen_construct(U, C);

    mapi_rotate(U, 3);
    mapi_pop(U, 2);

    ml_size count = mapi_vector_len(U);
    for (ml_size i = 0; i < count; i++) {
        mapi_vector_get(U, i);
        mapi_push_sio_io(U);
        mapi_rotate(U, 2);

        mcapi_decompile(U);

        mapi_pop(U, 1);
        mapi_sio_print(U, "\n");
        mapi_pop(U, 1);
    }
}
