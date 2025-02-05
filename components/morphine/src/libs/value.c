//
// Created by whyiskra on 28.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void toint(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                ml_size base = mapi_get_size(U, "base");

                mapi_push_arg(U, 0);
                mapi_to_based_integer(U, base);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_to_integer(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void todec(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            maux_nb_return();
    maux_nb_end
}

static void tobool(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_boolean(U);
            maux_nb_return();
    maux_nb_end
}

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_builtin_test(U, MTYPE_METAFIELD_TO_STRING)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    maux_nb_call(1, 1);
                } else {
                    maux_nb_return();
                }
            } else {
                mapi_to_string(U);
                maux_nb_return();
            }
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void compare(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);

            if (mapi_metatable_builtin_test(U, MTYPE_METAFIELD_COMPARE)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    mapi_push_arg(U, 1);
                    mapi_call(U, 2);
                } else {
                    maux_nb_return();
                }
            } else {
                mapi_push_arg(U, 1);
                int result = mapi_compare(U);
                mapi_push_integer(U, result);
                maux_nb_return();
            }
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void hash(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_builtin_test(U, MTYPE_METAFIELD_HASH)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    mapi_call(U, 1);
                } else {
                    maux_nb_return();
                }
            } else {
                ml_hash hash = mapi_hash(U);
                mapi_push_stringf(U, "%0*"MLIMIT_HASH_PR, (int) sizeof(ml_hash) * 2, hash);
                maux_nb_return();
            }
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void serialize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
            } else {
                maux_expect_args(U, 1);

                mapi_push_table(U);
                mapi_push_boolean(U, false);
                maux_table_set(U, "pretty");
                mapi_push_integer(U, 0);
                maux_table_set(U, "level");
                mapi_push_integer(U, 4);
                maux_table_set(U, "tab");
            }

            mapi_push_arg(U, 0);
            if (mapi_is(U, "table")) {
                maux_library_access(U, "value.serializetype.table");
                mapi_rotate(U, 2);
                mapi_peek(U, 2);
                maux_nb_call(2, 1);
            } else if (mapi_is(U, "vector")) {
                maux_library_access(U, "value.serializetype.vector");
                mapi_rotate(U, 2);
                mapi_peek(U, 2);
                maux_nb_call(2, 1);
            } else if (mapi_is(U, "string")) {
                maux_library_access(U, "value.serializetype.string");
                mapi_rotate(U, 2);
                maux_nb_call(1, 1);
            } else {
                maux_library_access(U, "value.tostr");
                mapi_rotate(U, 2);
                maux_nb_call(1, 1);
            }
        maux_nb_state(1);
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void tabulation(morphine_coroutine_t U, ml_size level, ml_size size) {
    mapi_push_string(U, "");
    for (ml_size i = 0; i < size; i++) {
        mapi_push_string(U, " ");
        mapi_string_concat(U);
    }

    mapi_push_string(U, "");
    for (ml_size i = 0; i < level; i++) {
        mapi_peek(U, 1);
        mapi_string_concat(U);
    }

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

static void serializetype_table(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool pretty = false;
            ml_size level = 0;
            ml_size tab = 4;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);

                if (maux_table_has(U, "pretty")) {
                    maux_table_get(U, "pretty");
                    pretty = mapi_get_boolean(U);
                    mapi_pop(U, 1);
                }

                if (maux_table_has(U, "level")) {
                    maux_table_get(U, "level");
                    level = mapi_get_size(U, "level");
                    mapi_pop(U, 1);
                }

                if (maux_table_has(U, "tab")) {
                    maux_table_get(U, "tab");
                    tab = mapi_get_size(U, NULL);
                    mapi_pop(U, 1);
                }

                mapi_pop(U, 1);
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_table(U);
            mapi_push_boolean(U, pretty);
            maux_table_set(U, "pretty");
            mapi_push_integer(U, level + 1);
            maux_table_set(U, "level");
            mapi_push_integer(U, tab);
            maux_table_set(U, "tab");

            mapi_push_string(U, pretty ? "{\n" : "{");

            mapi_push_arg(U, 0);
            if (mapi_table_len(U) == 0) {
                mapi_push_string(U, pretty ? "{ }" : "{}");
                maux_nb_return();
            }

            mapi_push_iterator(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mapi_iterator_init(U);
        maux_nb_state(1);
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);

                mapi_peek(U, 1);
                maux_table_get(U, "pretty");
                bool pretty = mapi_get_boolean(U);
                mapi_pop(U, 1);
                maux_table_get(U, "level");
                ml_size level = mapi_get_size(U, "level");
                mapi_pop(U, 1);
                maux_table_get(U, "tab");
                ml_size tab = mapi_get_size(U, NULL);
                mapi_pop(U, 2);

                if (pretty) {
                    tabulation(U, level - 1, tab);
                    mapi_push_string(U, "}");
                    mapi_string_concat(U);
                } else {
                    mapi_push_string(U, "}");
                }
                mapi_string_concat(U);
                maux_nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);

            maux_library_access(U, "value.serialize");
            mapi_rotate(U, 2);
            mapi_peek(U, 5);
            maux_nb_call(2, 2);
        maux_nb_state(2);
            mapi_push_result(U);
            mapi_rotate(U, 2);

            maux_library_access(U, "value.serialize");
            mapi_rotate(U, 2);
            mapi_peek(U, 5);
            maux_nb_call(2, 3);
        maux_nb_state(3);
            mapi_push_result(U);

            mapi_peek(U, 4);
            maux_table_get(U, "pretty");
            bool pretty = mapi_get_boolean(U);
            mapi_pop(U, 1);
            maux_table_get(U, "level");
            ml_size level = mapi_get_size(U, "level");
            mapi_pop(U, 1);
            maux_table_get(U, "tab");
            ml_size tab = mapi_get_size(U, NULL);
            mapi_pop(U, 2);

            mapi_rotate(U, 2);
            mapi_push_string(U, pretty ? " = " : "=");
            mapi_string_concat(U);
            mapi_rotate(U, 2);
            mapi_string_concat(U);

            mapi_rotate(U, 2);
            bool has_next = mapi_iterator_has(U);
            mapi_rotate(U, 2);

            if (pretty) {
                tabulation(U, level, tab);
                mapi_rotate(U, 2);
                mapi_string_concat(U);

                mapi_push_string(U, has_next ? ",\n" : "\n");
                mapi_string_concat(U);
            } else if (has_next) {
                mapi_push_string(U, ",");
                mapi_string_concat(U);
            }

            mapi_rotate(U, 2);
            mapi_rotate(U, 3);
            mapi_string_concat(U);
            mapi_rotate(U, 2);
            maux_nb_im_continue(1);
    maux_nb_end
}

static void serializetype_vector(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            bool pretty = false;
            ml_size level = 0;
            ml_size tab = 4;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);

                if (maux_table_has(U, "pretty")) {
                    maux_table_get(U, "pretty");
                    pretty = mapi_get_boolean(U);
                    mapi_pop(U, 1);
                }

                if (maux_table_has(U, "level")) {
                    maux_table_get(U, "level");
                    level = mapi_get_size(U, "level");
                    mapi_pop(U, 1);
                }

                if (maux_table_has(U, "tab")) {
                    maux_table_get(U, "tab");
                    tab = mapi_get_size(U, NULL);
                    mapi_pop(U, 1);
                }

                mapi_pop(U, 1);
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_table(U);
            mapi_push_boolean(U, pretty);
            maux_table_set(U, "pretty");
            mapi_push_integer(U, level + 1);
            maux_table_set(U, "level");
            mapi_push_integer(U, tab);
            maux_table_set(U, "tab");

            mapi_push_string(U, pretty ? "[\n" : "[");

            mapi_push_arg(U, 0);
            if (mapi_vector_len(U) == 0) {
                mapi_push_string(U, pretty ? "[ ]" : "[]");
                maux_nb_return();
            }

            mapi_push_iterator(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mapi_iterator_init(U);
        maux_nb_state(1);
            if (!mapi_iterator_has(U)) {
                mapi_pop(U, 1);

                mapi_peek(U, 1);
                maux_table_get(U, "pretty");
                bool pretty = mapi_get_boolean(U);
                mapi_pop(U, 1);
                maux_table_get(U, "level");
                ml_size level = mapi_get_size(U, "level");
                mapi_pop(U, 1);
                maux_table_get(U, "tab");
                ml_size tab = mapi_get_size(U, NULL);
                mapi_pop(U, 2);

                if (pretty) {
                    tabulation(U, level - 1, tab);
                    mapi_push_string(U, "]");
                    mapi_string_concat(U);
                } else {
                    mapi_push_string(U, "]");
                }
                mapi_string_concat(U);
                maux_nb_return();
            }

            mapi_iterator_next(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            maux_library_access(U, "value.serialize");
            mapi_rotate(U, 2);
            mapi_peek(U, 4);
            maux_nb_call(2, 2);
        maux_nb_state(2);
            mapi_push_result(U);

            mapi_peek(U, 3);
            maux_table_get(U, "pretty");
            bool pretty = mapi_get_boolean(U);
            mapi_pop(U, 1);
            maux_table_get(U, "level");
            ml_size level = mapi_get_size(U, "level");
            mapi_pop(U, 1);
            maux_table_get(U, "tab");
            ml_size tab = mapi_get_size(U, NULL);
            mapi_pop(U, 2);

            mapi_rotate(U, 2);
            bool has_next = mapi_iterator_has(U);
            mapi_rotate(U, 2);

            if (pretty) {
                tabulation(U, level, tab);
                mapi_rotate(U, 2);
                mapi_string_concat(U);

                mapi_push_string(U, has_next ? ",\n" : "\n");
                mapi_string_concat(U);
            } else if (has_next) {
                mapi_push_string(U, ",");
                mapi_string_concat(U);
            }

            mapi_rotate(U, 2);
            mapi_rotate(U, 3);
            mapi_string_concat(U);
            mapi_rotate(U, 2);
            maux_nb_im_continue(1);
    maux_nb_end
}

static void serializetype_string(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_string(U, "'");
            mapi_push_arg(U, 0);
            mapi_string_concat(U);

            mapi_push_string(U, "'");
            mapi_string_concat(U);

            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("toint", toint),
    MAUX_CONSTRUCT_FUNCTION("todec", todec),
    MAUX_CONSTRUCT_FUNCTION("tobool", tobool),
    MAUX_CONSTRUCT_FUNCTION("tostr", tostr),
    MAUX_CONSTRUCT_FUNCTION("compare", compare),
    MAUX_CONSTRUCT_FUNCTION("hash", hash),
    MAUX_CONSTRUCT_FUNCTION("serialize", serialize),
    MAUX_CONSTRUCT_FUNCTION("serializetype.table", serializetype_table),
    MAUX_CONSTRUCT_FUNCTION("serializetype.vector", serializetype_vector),
    MAUX_CONSTRUCT_FUNCTION("serializetype.string", serializetype_string),
    MAUX_CONSTRUCT_INTEGER("limits.intmin", mm_typemin(ml_integer)),
    MAUX_CONSTRUCT_INTEGER("limits.intmax", mm_typemax(ml_integer)),
    MAUX_CONSTRUCT_INTEGER("limits.sizemax", mm_typemax(ml_size)),

#define def_type(s)                MAUX_CONSTRUCT_STRING("type."#s, #s),
#define mspec_type_object(i, n, s) def_type(s)
#define mspec_type_value(i, n, s)  def_type(s)

#include "morphine/core/type/specification.h"

#undef def_type
#undef mspec_type_object
#undef mspec_type_value

    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_value(void) {
    return (morphine_library_t) {
        .name = "value",
        .sharedkey = NULL,
        .init = library_init
    };
}
