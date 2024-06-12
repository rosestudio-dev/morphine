//
// Created by why-iskra on 08.06.2024.
//

#include <string.h>
#include "morphinec/compiler.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/codegen.h"
#include "morphinec/printer.h"

MORPHINE_API void mcapi_compile(
    morphine_coroutine_t U,
    const char *main_name,
    const char *str,
    size_t size
) {
    struct strtable *T = strtable(U);
    strtable_index_t name = strtable_record(U, T, main_name, strlen(main_name));

    struct ast *A = ast(U, name);

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

//    mapi_push_sio_io(U);
//    printer_strtable(U, T);
//    printer_ast(U, T, A);
//    mapi_pop(U, 1);
}
