//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void rawget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);

            mapi_table_get(U);
            maux_nb_return();
    maux_nb_end
}

static void rawset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 2);

            mapi_table_set(U);
            maux_nb_return();
    maux_nb_end
}

static void idxget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            mapi_table_idx_get(U, index);
            maux_nb_return();
    maux_nb_end
}

static void idxkey(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            mapi_table_idx_key(U, index);
            maux_nb_return();
    maux_nb_end
}

static void idxset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            mapi_push_arg(U, 2);

            mapi_table_idx_set(U, index);
            maux_nb_return();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_clear(U);
            maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_copy(U);
            maux_nb_return();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);

            bool has = mapi_table_get(U);
            mapi_push_boolean(U, has);
            maux_nb_return();
    maux_nb_end
}

static void remove_(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);

            mapi_table_remove(U);
            maux_nb_return();
    maux_nb_end
}

static void mutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool value = true;
            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, "table");
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, "boolean");
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, "table");
            }

            mapi_table_mode_mutable(U, value);
            maux_nb_return();
    maux_nb_end
}

static void fixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool value = true;
            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, "table");
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, "boolean");
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, "table");
            }

            mapi_table_mode_fixed(U, value);
            maux_nb_return();
    maux_nb_end
}

static void lockmetatable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_mode_lock_metatable(U);
            maux_nb_return();
    maux_nb_end
}

static void lock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_mode_lock(U);
            maux_nb_return();
    maux_nb_end
}

static void ismutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void islocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void metatableislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_metatable_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_string(U, "{");

            mapi_peek(U, 1);
            mapi_iterator(U);
            mapi_iterator_init(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
        maux_nb_state(1)
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);
                mapi_push_string(U, "}");
                mapi_string_concat(U);
                maux_nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);

            mapi_library(U, "value.tostr", false);
            mapi_rotate(U, 2);
            mapi_call(U, 1);
        maux_nb_state(2)
            mapi_push_result(U);
            mapi_push_string(U, " = ");
            mapi_string_concat(U);
            mapi_peek(U, 1);

            mapi_library(U, "value.tostr", false);
            mapi_rotate(U, 2);
            mapi_call(U, 1);
        maux_nb_state(3)
            mapi_push_result(U);
            mapi_string_concat(U);

            mapi_rotate(U, 4);
            mapi_pop(U, 1);
            mapi_rotate(U, 3);
            mapi_rotate(U, 2);
            mapi_string_concat(U);
            mapi_rotate(U, 2);

            if (mapi_iterator_has(U)) {
                mapi_rotate(U, 2);
                mapi_push_string(U, ", ");
                mapi_string_concat(U);
                mapi_rotate(U, 2);
            }

            maux_nb_continue(1);
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "rawget",            rawget },
    { "rawset",            rawset },
    { "idxget",            idxget },
    { "idxkey",            idxkey },
    { "idxset",            idxset },
    { "clear",             clear },
    { "copy",              copy },
    { "has",               has },
    { "remove",            remove_ },
    { "mutable",           mutable },
    { "fixed",             fixed },
    { "lockmetatable",     lockmetatable },
    { "lock",              lock },
    { "isfixed",           isfixed },
    { "ismutable",         ismutable },
    { "metatableislocked", metatableislocked },
    { "islocked",          islocked },
    { "tostr",             tostr },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "table",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_table(void) {
    return &library;
}
