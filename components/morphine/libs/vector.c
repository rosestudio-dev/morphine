//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void clear(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_clear(U);
            nb_return();
    nb_end
}

static void copy(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_copy(U);
            nb_return();
    nb_end
}

static void resize(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,integer", "vector,integer");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            ml_size size = mapi_get_size(U);
            mapi_pop(U, 1);

            mapi_vector_resize(U, size);
            nb_return();
    nb_end
}

static void add(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,integer,any", "vector,integer,any");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            ml_size index = mapi_get_index(U);
            mapi_pop(U, 1);

            if (variant == 0) {
                mapi_push_arg(U, 1);
            } else {
                mapi_push_arg(U, 2);
            }

            mapi_vector_add(U, index);
            nb_return();
    nb_end
}

static void remove_(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,integer,any", "vector,integer,any");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            ml_size index = mapi_get_index(U);
            mapi_pop(U, 1);

            if (variant == 0) {
                mapi_push_arg(U, 1);
            } else {
                mapi_push_arg(U, 2);
            }

            mapi_vector_remove(U, index);
            nb_return();
    nb_end
}

static void push(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,any", "vector,any");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            mapi_vector_push(U);
            nb_return();
    nb_end
}

static void peek(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_peek(U);
            nb_return();
    nb_end
}

static void pop(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_pop(U);
            nb_return();
    nb_end
}

static void frontpush(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,any", "vector,any");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            mapi_vector_push_front(U);
            nb_return();
    nb_end
}

static void frontpeek(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_peek_front(U);
            nb_return();
    nb_end
}

static void frontpop(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_pop_front(U);
            nb_return();
    nb_end
}

static void setmutable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,boolean", "vector,boolean");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_vector_mode_mutable(U, value);
            nb_return();
    nb_end
}

static void setfixed(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector,boolean", "vector,boolean");
            if (variant == 0) {
                mapi_push_self(U);
                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                mapi_push_arg(U, 1);
            }

            bool value = mapi_get_boolean(U);
            mapi_pop(U, 1);

            mapi_vector_mode_fixed(U, value);
            nb_return();
    nb_end
}

static void lock(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_vector_mode_lock(U);
            nb_return();
    nb_end
}

static void ismutable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            bool value = mapi_vector_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void isfixed(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            bool value = mapi_vector_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void islocked(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            bool value = mapi_vector_mode_is_locked(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static void tostr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:vector", "vector");
            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_push_string(U, "[");

            mapi_peek(U, 1);
            mapi_iterator(U);
            mapi_iterator_init(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
        nb_state(1)
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);
                mapi_push_string(U, "]");
                mapi_string_concat(U);
                nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mlib_value_call(U, "tostr", 1);
        nb_state(2)
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

            nb_continue(1);
    nb_end
}

static struct maux_construct_field table[] = {
    { "clear",      clear },
    { "copy",       copy },
    { "resize",     resize },
    { "add",        add },
    { "remove",     remove_ },
    { "push",       push },
    { "peek",       peek },
    { "pop",        pop },
    { "frontpush",  frontpush },
    { "frontpeek",  frontpeek },
    { "frontpop",   frontpop },
    { "setmutable", setmutable },
    { "setfixed",   setfixed },
    { "lock",       lock },
    { "isfixed",    isfixed },
    { "ismutable",  ismutable },
    { "islocked",   islocked },
    { "tostr",      tostr },
    { NULL, NULL }
};

void mlib_vector_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "vector.");
}

MORPHINE_LIB void mlib_vector_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "vector.", name, argc);
}
