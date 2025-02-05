//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void rawget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);

            mapi_table_get(U);
            maux_nb_return();
    maux_nb_end
}

static void rawset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 2);

            mapi_table_set(U);
            maux_nb_return();
    maux_nb_end
}

static void idxget(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            mapi_table_idx_get(U, index);
            maux_nb_return();
    maux_nb_end
}

static void idxkey(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            mapi_table_idx_key(U, index);
            maux_nb_return();
    maux_nb_end
}

static void idxset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            mapi_push_arg(U, 2);

            mapi_table_idx_set(U, index);
            maux_nb_return();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_table_clear(U);
            maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_table_copy(U);
            maux_nb_return();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_push_arg(U, 1);

            bool result = mapi_table_has(U);
            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void remove_(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);
            mapi_push_arg(U, 1);

            mapi_table_remove(U);
            maux_nb_return();
    maux_nb_end
}

static void mutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_mutable(U, value);
            maux_nb_return();
    maux_nb_end
}

static void immutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_mutable(U, false);
            maux_nb_return();
    maux_nb_end
}

static void fixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_fixed(U, value);
            maux_nb_return();
    maux_nb_end
}

static void unfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_fixed(U, false);
            maux_nb_return();
    maux_nb_end
}

static void accessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool value = true;
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_accessible(U, value);
            maux_nb_return();
    maux_nb_end
}

static void inaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 0) {
                mapi_push_table(U);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_TABLE);
            }

            mapi_table_mode_accessible(U, false);
            maux_nb_return();
    maux_nb_end
}

static void lockmetatable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_table_lock_metatable(U);
            maux_nb_return();
    maux_nb_end
}

static void lockmode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_table_lock_mode(U);
            maux_nb_return();
    maux_nb_end
}

static void ismutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            bool value = mapi_table_mode_is_mutable(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isfixed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            bool value = mapi_table_mode_is_fixed(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            bool value = mapi_table_mode_is_accessible(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void metatableislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            bool value = mapi_table_metatable_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void modeislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            bool value = mapi_table_mode_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("rawget", rawget),
    MAUX_CONSTRUCT_FUNCTION("rawset", rawset),
    MAUX_CONSTRUCT_FUNCTION("idxget", idxget),
    MAUX_CONSTRUCT_FUNCTION("idxkey", idxkey),
    MAUX_CONSTRUCT_FUNCTION("idxset", idxset),
    MAUX_CONSTRUCT_FUNCTION("clear", clear),
    MAUX_CONSTRUCT_FUNCTION("copy", copy),
    MAUX_CONSTRUCT_FUNCTION("has", has),
    MAUX_CONSTRUCT_FUNCTION("remove", remove_),
    MAUX_CONSTRUCT_FUNCTION("mutable", mutable),
    MAUX_CONSTRUCT_FUNCTION("immutable", immutable),
    MAUX_CONSTRUCT_FUNCTION("fixed", fixed),
    MAUX_CONSTRUCT_FUNCTION("unfixed", unfixed),
    MAUX_CONSTRUCT_FUNCTION("accessible", accessible),
    MAUX_CONSTRUCT_FUNCTION("inaccessible", inaccessible),
    MAUX_CONSTRUCT_FUNCTION("lockmetatable", lockmetatable),
    MAUX_CONSTRUCT_FUNCTION("lockmode", lockmode),
    MAUX_CONSTRUCT_FUNCTION("isfixed", isfixed),
    MAUX_CONSTRUCT_FUNCTION("ismutable", ismutable),
    MAUX_CONSTRUCT_FUNCTION("isaccessible", isaccessible),
    MAUX_CONSTRUCT_FUNCTION("metatableislocked", metatableislocked),
    MAUX_CONSTRUCT_FUNCTION("modeislocked", modeislocked),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_table(void) {
    return (morphine_library_t) {
        .name = "table",
        .sharedkey = NULL,
        .init = library_init
    };
}
