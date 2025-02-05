//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

#define SHAREDKEY ("morphine")

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            bool result = mapi_sharedstorage_get(U, SHAREDKEY);
            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_sharedstorage_get(U, SHAREDKEY);
            maux_nb_return();
    maux_nb_end
}

static void set(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            mapi_push_arg(U, 1);
            mapi_sharedstorage_set(U, SHAREDKEY);
            maux_nb_leave();
    maux_nb_end
}

static void remove(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_sharedstorage_remove(U, SHAREDKEY);
            maux_nb_return();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);

            mapi_sharedstorage_clear(U, SHAREDKEY);
            maux_nb_leave();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("has", has),
    MAUX_CONSTRUCT_FUNCTION("get", get),
    MAUX_CONSTRUCT_FUNCTION("set", set),
    MAUX_CONSTRUCT_FUNCTION("remove", remove),
    MAUX_CONSTRUCT_FUNCTION("clear", clear),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_sharedstorage(void) {
    return (morphine_library_t) {
        .name = "sharedstorage",
        .sharedkey = SHAREDKEY,
        .init = library_init
    };
}
