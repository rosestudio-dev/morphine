//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/lib.h"
#include "morphinec/compiler.h"
#include "morphinec/decompiler.h"

static void string(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            const char *main_name;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 0);
                main_name = mapi_get_string(U);
                mapi_push_arg(U, 1);
            } else {
                maux_expect_args(U, 1);
                main_name = "compiled";
                mapi_push_arg(U, 0);
            }

            const char *text = mapi_get_string(U);
            size_t size = mapi_string_len(U);

            mcapi_compile(U, main_name, text, size);
            nb_return();
    nb_end
}

static void decompile(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);

            ml_size count;
            if (mapi_is_type(U, "vector")) {
                count = mapi_vector_len(U);
            } else {
                mapi_push_vector(U, 1);
                mapi_rotate(U, 2);
                mapi_vector_set(U, 0);
                count = 1;
            }

            for (ml_size i = 0; i < count; i++) {
                mapi_vector_get(U, i);
                mapi_push_arg(U, 0);
                mapi_rotate(U, 2);

                mapi_extract_callable(U);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mcapi_decompile(U);

                mapi_pop(U, 1);
                mapi_sio_print(U, "\n");
                mapi_pop(U, 1);
            }
            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "string",    string },
    { "decompile", decompile },
    { NULL, NULL }
};

MORPHINE_LIB void mclib_compiler_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "compiler.");
}

MORPHINE_LIB void mclib_compiler_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "compiler.", name, argc);
}