//
// Created by why-iskra on 02.06.2024.
//

#include <morphine.h>
#include "morphine/libs/loader.h"
#include "morphine/compiler/strtable.h"
#include "morphine/compiler/lex.h"
#include "morphine/compiler/parser.h"
#include "morphine/compiler/printer.h"

static void string(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *text = mapi_get_string(U);
            size_t size = mapi_string_len(U);

            strtable(U);
            lex(U, text, size);
            parser(U);
            while (parser_next(U)) { }
            mapi_rotate(U, 5);
            mapi_pop(U, 2);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
            mapi_rotate(U, 2);

            mapi_rotate(U, 2);
            printer_strtable(U);

            mapi_rotate(U, 2);
            printer_ast(U);

            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "string", string },
    { NULL, NULL }
};

void mlib_compiler_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "compiler.");
}

MORPHINE_LIB void mlib_compiler_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "compiler.", name, argc);
}