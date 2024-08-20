//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/lib.h"
#include "morphinec/compiler.h"
#include "morphinec/disassembler.h"

#define DEFAULT_MAIN_NAME "compiled"

static void compile(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            bool vector = false;
            const char *name = DEFAULT_MAIN_NAME;
            if (mapi_is_type(U, "table")) {
                mapi_push_string(U, "name");
                mapi_table_get(U);
                if (!mapi_is_type(U, "nil")) {
                    name = mapi_get_string(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "vector");
                mapi_table_get(U);
                if (!mapi_is_type(U, "nil")) {
                    vector = mapi_get_boolean(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "text");
                mapi_table_get(U);
            }

            mcapi_compile(U, name, vector);
            maux_nb_return();
    maux_nb_end
}

static void disassembly(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
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

                mcapi_disassembly(U);

                mapi_push_arg(U, 0);
                mapi_sio_print(U, "\n");
                mapi_pop(U, 1);
            }
            maux_nb_leave();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "compile",     compile },
    { "disassembly", disassembly },

    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "compiler",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mclib_compiler(void) {
    return &library;
}
