//
// Created by why-iskra on 24.09.2024.
//

#include "library.h"
#include "env.h"
#include "scripts.h"

#ifdef USE_READLINE

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define api_initreadline(I)   ((void) I, rl_readline_name = "morphine")

static void api_readline(morphine_coroutine_t U, const char *promt) {
    char *result = readline(promt);
    mapi_push_string(U, result);
    add_history(result);
    free(result);
}
#else
#define api_initreadline(I) ((void) I)

static void api_readline(morphine_coroutine_t U, const char *promt) {
    mapi_push_sio_io(U);
    mapi_sio_print(U, promt);
    mapi_sio_flush(U);

    maux_sio_read_line(U);
}
#endif

static void lib_exit(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            ml_integer code = mapi_get_integer(U);
            env_exit(mapi_instance(U), code);
    maux_nb_end
}

static void lib_readline(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *promt = mapi_get_cstr(U);

            api_readline(U, promt);
            maux_nb_return();
    maux_nb_end
}

static void lib_getscript(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *name = mapi_get_cstr(U);

            const struct scripts_raw_data *data = scripts_data_table;
            for (; data->name != NULL; data++) {
                if (strcmp(data->name, name) == 0) {
                    break;
                }
            }

            if (data->name == NULL) {
                mapi_error(U, "script not found");
            }

            mapi_push_stringn(U, (const char *) data->data, data->size);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("exit", lib_exit),
    MAUX_CONSTRUCT_FUNCTION("readline", lib_readline),
    MAUX_CONSTRUCT_FUNCTION("getscript", lib_getscript),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    api_initreadline(mapi_instance(U));
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_launcher(void) {
    return (morphine_library_t) {
        .name = "__app",
        .sharedkey = NULL,
        .init = library_init
    };
}
