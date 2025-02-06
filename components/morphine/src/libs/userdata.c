//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void hastype(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            const char *type = mapi_get_cstr(U);
            bool has = mapi_usertype_has(U, type);
            mapi_push_boolean(U, has);
            maux_nb_return();
    maux_nb_end
}

static void istyped(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            bool value = mapi_userdata_is_typed(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("hastype", hastype),
    MAUX_CONSTRUCT_FUNCTION("istyped", istyped),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_userdata(void) {
    return (morphine_library_t) {
        .name = "userdata",
        .sharedkey = NULL,
        .init = library_init
    };
}
