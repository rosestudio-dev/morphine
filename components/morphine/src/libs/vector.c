//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            ml_size size = mapi_get_size(U, NULL);

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
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_clear(U);
            maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_copy(U);
            maux_nb_return();
    maux_nb_end
}

static void trim(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 1);
            ml_size from = mapi_get_size(U, "index");

            mapi_push_arg(U, 2);
            ml_size to = mapi_get_size(U, "index");

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_push_vector(U, from <= to ? to - from : 0);

            for (ml_size i = from; i < to; i++) {
                mapi_peek(U, 1);
                mapi_vector_get(U, i);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mapi_vector_set(U, i - from);
            }

            maux_nb_return();
    maux_nb_end
}

static void sort(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_sort(U);
            maux_nb_return();
    maux_nb_end
}

static void resize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U, NULL);
            mapi_pop(U, 1);

            mapi_vector_resize(U, size);
            maux_nb_return();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_push_arg(U, 1);

            bool result = mapi_vector_has(U);
            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void add(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            mapi_push_arg(U, 2);

            mapi_vector_add(U, index);
            maux_nb_return();
    maux_nb_end
}

static void remove_(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            mapi_vector_remove(U, index);
            maux_nb_return();
    maux_nb_end
}

static void push(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);
            mapi_push_arg(U, 1);

            mapi_vector_push(U);
            maux_nb_return();
    maux_nb_end
}

static void peek(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_peek(U);
            maux_nb_return();
    maux_nb_end
}

static void pop(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_pop(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpush(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);
            mapi_push_arg(U, 1);

            mapi_vector_push_front(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpeek(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_peek_front(U);
            maux_nb_return();
    maux_nb_end
}

static void frontpop(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_pop_front(U);
            maux_nb_return();
    maux_nb_end
}

static void mutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_mutable(U, value);
            maux_nb_return();
    maux_nb_end
}

static void immutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_mutable(U, false);
            maux_nb_return();
    maux_nb_end
}

static void fixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } else if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_fixed(U, value);
            maux_nb_return();
    maux_nb_end
}

static void unfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_fixed(U, false);
            maux_nb_return();
    maux_nb_end
}

static void accessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } else if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_accessible(U, value);
            maux_nb_return();
    maux_nb_end
}

static void inaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_vector(U, 0);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_VECTOR);
            }

            mapi_vector_mode_accessible(U, false);
            maux_nb_return();
    maux_nb_end
}

static void lockmode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            mapi_vector_lock_mode(U);
            maux_nb_return();
    maux_nb_end
}

static void ismutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            bool value = mapi_vector_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            bool value = mapi_vector_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            bool value = mapi_vector_mode_is_accessible(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void modeislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_VECTOR);

            bool value = mapi_vector_mode_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("create", create),
    MAUX_CONSTRUCT_FUNCTION("clear", clear),
    MAUX_CONSTRUCT_FUNCTION("copy", copy),
    MAUX_CONSTRUCT_FUNCTION("trim", trim),
    MAUX_CONSTRUCT_FUNCTION("sort", sort),
    MAUX_CONSTRUCT_FUNCTION("resize", resize),
    MAUX_CONSTRUCT_FUNCTION("has", has),
    MAUX_CONSTRUCT_FUNCTION("add", add),
    MAUX_CONSTRUCT_FUNCTION("remove", remove_),
    MAUX_CONSTRUCT_FUNCTION("push", push),
    MAUX_CONSTRUCT_FUNCTION("peek", peek),
    MAUX_CONSTRUCT_FUNCTION("pop", pop),
    MAUX_CONSTRUCT_FUNCTION("frontpush", frontpush),
    MAUX_CONSTRUCT_FUNCTION("frontpeek", frontpeek),
    MAUX_CONSTRUCT_FUNCTION("frontpop", frontpop),
    MAUX_CONSTRUCT_FUNCTION("mutable", mutable),
    MAUX_CONSTRUCT_FUNCTION("immutable", immutable),
    MAUX_CONSTRUCT_FUNCTION("fixed", fixed),
    MAUX_CONSTRUCT_FUNCTION("unfixed", unfixed),
    MAUX_CONSTRUCT_FUNCTION("accessible", accessible),
    MAUX_CONSTRUCT_FUNCTION("inaccessible", inaccessible),
    MAUX_CONSTRUCT_FUNCTION("lockmode", lockmode),
    MAUX_CONSTRUCT_FUNCTION("isfixed", isfixed),
    MAUX_CONSTRUCT_FUNCTION("ismutable", ismutable),
    MAUX_CONSTRUCT_FUNCTION("isaccessible", isaccessible),
    MAUX_CONSTRUCT_FUNCTION("modeislocked", modeislocked),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_vector(void) {
    return (morphine_library_t) {
        .name = "vector",
        .sharedkey = NULL,
        .init = library_init
    };
}
