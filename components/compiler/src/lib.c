//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/lib.h"
#include "morphinec/strtable.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/printer.h"
#include "morphinec/codegen.h"
#include "morphinec/compiler.h"

static void string(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *text = mapi_get_string(U);
            size_t size = mapi_string_len(U);

            mcapi_compile(U, text, size);

            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "string", string },
    { NULL, NULL }
};

MORPHINE_LIB void mclib_compiler_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "compiler.");
}

MORPHINE_LIB void mclib_compiler_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "compiler.", name, argc);
}