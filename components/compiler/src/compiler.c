//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/compiler.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/printer.h"
#include "morphinec/codegen.h"

MORPHINE_API void mcapi_compile(morphine_coroutine_t U, const char *str, size_t size) {
    strtable(U);
    lex(U, str, size);
    parser(U);
    while (parser_step(U)) { }
    mapi_rotate(U, 5);
    mapi_pop(U, 2);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);
    mapi_rotate(U, 2);

    mapi_rotate(U, 2);
    printer_strtable(U);

    mapi_rotate(U, 2);
    printer_ast(U);

    codegen(U);
}
