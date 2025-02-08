//
// Created by whyiskra on 30.12.23.
//

#include "morphine/libs/builtin.h"
#include <morphine.h>

static void rawset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 3);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);
        mapi_push_arg(U, 2);

        mapi_table_set(U);
        maux_nb_return();
    maux_nb_end
}

static void rawget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);

        mapi_table_get(U);
        maux_nb_return();
    maux_nb_end
}

static void rawrmv(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);

        mapi_table_remove(U);
        maux_nb_return();
    maux_nb_end
}

static void idxset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 3);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);
        ml_size index = mapi_get_size(U, "index");
        mapi_pop(U, 1);

        mapi_push_arg(U, 2);

        mapi_table_idx_set(U, index);
        maux_nb_return();
    maux_nb_end
}

static void idxget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);
        ml_size index = mapi_get_size(U, "index");
        mapi_pop(U, 1);

        mapi_table_idx_get(U, index);

        mapi_push_table(U);
        mapi_rotate(U, 2);
        maux_table_set(U, "value");

        mapi_rotate(U, 2);
        maux_table_set(U, "key");

        maux_nb_return();
    maux_nb_end
}

static void idxrmv(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);
        ml_size index = mapi_get_size(U, "index");
        mapi_pop(U, 1);

        mapi_table_idx_remove(U, index);

        mapi_push_table(U);
        mapi_rotate(U, 2);
        maux_table_set(U, "value");

        mapi_rotate(U, 2);
        maux_table_set(U, "key");

        maux_nb_return();
    maux_nb_end
}

static void first(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        if (mapi_table_first(U)) {
            mapi_push_table(U);
            mapi_rotate(U, 2);
            maux_table_set(U, "value");
            mapi_rotate(U, 2);
            maux_table_set(U, "key");
        } else {
            mapi_push_nil(U);
        }
        maux_nb_return();
    maux_nb_end
}

static void next(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);
        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);
        if (mapi_table_next(U)) {
            mapi_push_table(U);
            mapi_rotate(U, 2);
            maux_table_set(U, "value");
            mapi_rotate(U, 2);
            maux_table_set(U, "key");
        } else {
            mapi_push_nil(U);
        }
        maux_nb_return();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_table_clear(U);
        maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_table_copy(U);
        maux_nb_return();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);

        mapi_push_arg(U, 0);
        maux_expect(U, mstr_type_table);

        mapi_push_arg(U, 1);

        bool result = mapi_table_has(U);
        mapi_push_boolean(U, result);
        maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("rawget", rawget),
    MAUX_CONSTRUCT_FUNCTION("rawset", rawset),
    MAUX_CONSTRUCT_FUNCTION("rawrmv", rawrmv),
    MAUX_CONSTRUCT_FUNCTION("idxget", idxget),
    MAUX_CONSTRUCT_FUNCTION("idxset", idxset),
    MAUX_CONSTRUCT_FUNCTION("idxrmv", idxrmv),
    MAUX_CONSTRUCT_FUNCTION("first", first),
    MAUX_CONSTRUCT_FUNCTION("next", next),
    MAUX_CONSTRUCT_FUNCTION("clear", clear),
    MAUX_CONSTRUCT_FUNCTION("copy", copy),
    MAUX_CONSTRUCT_FUNCTION("has", has),
    MAUX_CONSTRUCT_END, //
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_table(void) {
    return (morphine_library_t) {
        .name = "table",
        .sharedkey = NULL,
        .init = library_init,
    };
}
