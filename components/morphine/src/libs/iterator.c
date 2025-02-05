//
// Created by why-iskra on 02.11.2024.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"

static void instance(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_nb_operation("iterator", 1);
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void init(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            mapi_push_arg(U, 1);
            mapi_push_arg(U, 2);
            maux_nb_operation("iteratorinit", 1);
        maux_nb_state(1);
            maux_nb_leave();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_nb_operation("iteratorhas", 1);
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void next(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_nb_operation("iteratornext", 1);
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void vectorize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) != 1) {
                maux_expect_args(U, 3);
            }

            mapi_push_arg(U, 0);
            maux_nb_operation("iterator", 1);
        maux_nb_state(1);
            mapi_push_result(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mapi_push_vector(U, 0);
            mapi_vector_mode_fixed(U, false);
            mapi_rotate(U, 2);

            if (mapi_args(U) == 1) {
                mapi_push_string(U, "key");
                mapi_push_string(U, "value");
            } else {
                mapi_push_arg(U, 1);
                mapi_push_arg(U, 2);
            }
            maux_nb_operation("iteratorinit", 2);
        maux_nb_state(2);
            maux_nb_operation("iteratorhas", 3);
        maux_nb_state(3);
            mapi_push_result(U);
            bool has = mapi_get_boolean(U);
            mapi_pop(U, 1);

            if (has) {
                maux_nb_operation("iteratornext", 4);
            } else {
                mapi_pop(U, 1);
                maux_nb_return();
            }
        maux_nb_state(4);
            mapi_push_result(U);
            mapi_peek(U, 2);
            mapi_rotate(U, 2);
            mapi_vector_push(U);
            mapi_pop(U, 1);
            maux_nb_im_continue(2);
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("instance", instance),
    MAUX_CONSTRUCT_FUNCTION("init", init),
    MAUX_CONSTRUCT_FUNCTION("has", has),
    MAUX_CONSTRUCT_FUNCTION("next", next),
    MAUX_CONSTRUCT_FUNCTION("vectorize", vectorize),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_iterator(void) {
    return (morphine_library_t) {
        .name = "iterator",
        .sharedkey = NULL,
        .init = library_init
    };
}

