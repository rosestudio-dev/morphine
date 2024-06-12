//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void rawget(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);

            mapi_table_get(U);
            nb_return();
    nb_end
}

static void rawset(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 2);

            mapi_table_set(U);
            nb_return();
    nb_end
}

static void clear(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_clear(U);
            nb_return();
    nb_end
}

static void copy(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_copy(U);
            nb_return();
    nb_end
}

static void has(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);

            bool has = mapi_table_get(U);
            mapi_push_boolean(U, has);
            nb_return();
    nb_end
}

static void remove_(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);

            mapi_table_remove(U);
            nb_return();
    nb_end
}

static void mutable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);
            maux_expect(U, "boolean");

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_table_mode_mutable(U, value);
            nb_return();
    nb_end
}

static void fixed(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");
            mapi_push_arg(U, 1);
            maux_expect(U, "boolean");

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_table_mode_fixed(U, value);
            nb_return();
    nb_end
}

static void lockmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_mode_lock_metatable(U);
            nb_return();
    nb_end
}

static void lock(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_table_mode_lock(U);
            nb_return();
    nb_end
}

static void ismutable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void isfixed(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void islocked(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_is_locked(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void metatableislocked(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            bool value = mapi_table_mode_metatable_is_locked(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void tostr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "table");

            mapi_push_string(U, "{");

            mapi_peek(U, 1);
            mapi_iterator(U);
            mapi_iterator_init(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
        nb_state(1)
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);
                mapi_push_string(U, "}");
                mapi_string_concat(U);
                nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);
            mlib_value_call(U, "tostr", 1);
        nb_state(2)
            mapi_push_result(U);
            mapi_push_string(U, " to ");
            mapi_string_concat(U);
            mapi_peek(U, 1);

            mlib_value_call(U, "tostr", 1);
        nb_state(3)
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

            nb_continue(1);
    nb_end
}

static struct maux_construct_field table[] = {
    { "rawget",            rawget },
    { "rawset",            rawset },
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

void mlib_table_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "table.");
}

MORPHINE_LIB void mlib_table_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "table.", name, argc);
}
