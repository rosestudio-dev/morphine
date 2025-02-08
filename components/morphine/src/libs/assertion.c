//
// Created by why-iskra on 14.12.2024.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void lib_assert(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool stub = false;
            if (mapi_args(U) == 1) {
                stub = true;
            } else {
                maux_expect_args(U, 2);
            }

            mapi_push_arg(U, 0);
            mapi_to_boolean(U);
            bool result = mapi_get_boolean(U);
            if (!result) {
                if (stub) {
                    mapi_error(U, "assertion failed");
                } else {
                    mapi_push_arg(U, 1);
                    mapi_error(U, NULL);
                }
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_nonnil(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            ml_size args = mapi_args(U);
            for (ml_size i = 0; i < args; i++) {
                mapi_push_arg(U, i);
                if (mapi_is_type(U, mstr_type_nil)) {
                    mapi_error(U, "expected non-nil value");
                }
                mapi_pop(U, 1);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_typecheck(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args_minimum(U, 2);

            mapi_push_arg(U, mapi_args(U) - 1);
            maux_nb_operation("type", 1);
        maux_nb_state(1);
            mapi_push_result(U);

            ml_size size = mapi_args(U) - 1;
            for (ml_size i = 0; i < size; i++) {
                mapi_push_arg(U, i);
                maux_expect(U, mstr_type_string);

                if (mapi_compare(U) == 0) {
                    maux_nb_leave();
                }

                mapi_pop(U, 1);
            }

            mapi_push_string(U, "expected ");
            for (ml_size i = 0; i < size; i++) {
                mapi_push_arg(U, i);

                if (size > 1 && i == size - 2) {
                    mapi_push_string(U, " or ");
                } else {
                    mapi_push_string(U, ", ");
                }

                mapi_string_concat(U);
                mapi_string_concat(U);
            }

            mapi_push_stringf(U, "but got ");
            mapi_string_concat(U);
            mapi_rotate(U, 2);
            mapi_to_string(U);
            mapi_string_concat(U);
            mapi_error(U, NULL);
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("assert", lib_assert),
    MAUX_CONSTRUCT_FUNCTION("nonnil", lib_nonnil),
    MAUX_CONSTRUCT_FUNCTION("typecheck", lib_typecheck),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_assertion(void) {
    return (morphine_library_t) {
        .name = "assertion",
        .sharedkey = NULL,
        .init = library_init
    };
}
