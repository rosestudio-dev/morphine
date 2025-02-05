//
// Created by whyiskra on 30.12.23.
//

#include "morphinel/subprocess.h"
#include "morphinel/process/spawn.h"

static void lib_spawn(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool env = false;
            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
            } else {
                maux_expect_args(U, 2);
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
                env = true;
            }

            mlapi_process_spawn(U, env);

            mapi_push_table(U);
            mapi_rotate(U, 2);
            maux_table_set(U, "err");
            mapi_rotate(U, 2);
            maux_table_set(U, "out");
            mapi_rotate(U, 2);
            maux_table_set(U, "in");
            mapi_rotate(U, 2);
            maux_table_set(U, "process");
        maux_nb_return();
    maux_nb_end
}

static void lib_wait(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            int status = mlapi_process_wait(U);
            mapi_push_integer(U, status);
        maux_nb_return();
    maux_nb_end
}

static void lib_kill(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool force = false;
            if(mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                force = mapi_get_boolean(U);

                mapi_push_arg(U, 0);
            }
            mlapi_process_kill(U, force);
        maux_nb_leave();
    maux_nb_end
}

static void lib_isalive(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool isalive = mlapi_process_isalive(U);
            mapi_push_boolean(U, isalive);
        maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("spawn", lib_spawn),
    MAUX_CONSTRUCT_FUNCTION("wait", lib_wait),
    MAUX_CONSTRUCT_FUNCTION("kill", lib_kill),
    MAUX_CONSTRUCT_FUNCTION("isalive", lib_isalive),
    MAUX_CONSTRUCT_END,
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_subprocess(void) {
    return (morphine_library_t) {
        .name = "subprocess",
        .sharedkey = NULL,
        .init = library_init,
    };
}
