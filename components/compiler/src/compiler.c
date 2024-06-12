//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/compiler.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/codegen.h"

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
}
