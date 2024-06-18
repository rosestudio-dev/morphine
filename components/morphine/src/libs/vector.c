//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "integer");
            ml_size size = mapi_get_size(U);

            mapi_push_vector(U, size);
            for (ml_size i = 0; i < size; i++) {
                mapi_push_arg(U, 1);
                mapi_vector_set(U, i);
            }
            maux_nb_return();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_clear(U);
            maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_copy(U);
            maux_nb_return();
    maux_nb_end
}

static void resize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);
            maux_expect(U, "integer");

            ml_size size = mapi_get_size(U);
            mapi_pop(U, 1);

            mapi_vector_resize(U, size);
            maux_nb_return();
    maux_nb_end
}

static void add(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);
            maux_expect(U, "integer");

            ml_size index = mapi_get_index(U);
            mapi_pop(U, 1);

            mapi_push_arg(U, 2);

            mapi_vector_add(U, index);
            maux_nb_return();
    maux_nb_end
}

static void remove_(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);
            maux_expect(U, "integer");

            ml_size index = mapi_get_index(U);
            mapi_pop(U, 1);

            mapi_vector_remove(U, index);
            maux_nb_return();
    maux_nb_end
}

static void push(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);

            mapi_vector_push(U);
            maux_nb_return();
    maux_nb_end
}

static void peek(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_peek(U);
            maux_nb_return();
    maux_nb_end
}

static void pop(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_pop(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpush(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);

            mapi_vector_push_front(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpeek(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_peek_front(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpop(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_pop_front(U);
            maux_nb_return();
    maux_nb_end
}

static void mutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);
            maux_expect(U, "boolean");

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_vector_mode_mutable(U, value);
            maux_nb_return();
    maux_nb_end
}

static void fixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");
            mapi_push_arg(U, 1);
            maux_expect(U, "boolean");

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_vector_mode_fixed(U, value);
            maux_nb_return();
    maux_nb_end
}

static void lock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_vector_mode_lock(U);
            maux_nb_return();
    maux_nb_end
}

static void ismutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            bool value = mapi_vector_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            bool value = mapi_vector_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void islocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            bool value = mapi_vector_mode_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "vector");

            mapi_push_string(U, "[");

            mapi_peek(U, 1);
            mapi_iterator(U);
            mapi_iterator_init(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
        maux_nb_state(1)
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);
                mapi_push_string(U, "]");
                mapi_string_concat(U);
                maux_nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            maux_library_get(U, "value", "tostr");
            mapi_calli(U, 1);
        maux_nb_state(2)
            mapi_push_result(U);
            mapi_rotate(U, 3);
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
    { "create",    create },
    { "clear",     clear },
    { "copy",      copy },
    { "resize",    resize },
    { "add",       add },
    { "remove",    remove_ },
    { "push",      push },
    { "peek",      peek },
    { "pop",       pop },
    { "frontpush", frontpush },
    { "frontpeek", frontpeek },
    { "frontpop",  frontpop },
    { "mutable",   mutable },
    { "fixed",     fixed },
    { "lock",      lock },
    { "isfixed",   isfixed },
    { "ismutable", ismutable },
    { "islocked",  islocked },
    { "tostr",     tostr },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "vector",
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_vector(void) {
    return &library;
}
