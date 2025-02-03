//
// Created by whyiskra on 30.12.23.
//

#include "morphinel/bigint.h"

#define BIGINT_WRAPPED_TYPE  "bigint-wrapped"

static void bigint_metatable_wrap(morphine_coroutine_t);

static struct mlib_bigint *bigint_from(morphine_coroutine_t U) {
    if (mapi_is_type(U, "integer")) {
        ml_integer integer = mapi_get_integer(U);
        return mlapi_bigint_from_integer(U, integer);
    } else if (mapi_is_type(U, "string")) {
        const char *string = mapi_get_cstr(U);
        return mlapi_bigint_from_string(U, string);
    } else if (mapi_is_type(U, MLIB_BIGINT_USERDATA_TYPE)) {
        return mlapi_get_bigint(U);
    }

    mapi_error(U, "bigint can only be created from string or integer");
}

static inline void raw_binary(
    morphine_coroutine_t U,
    struct mlib_bigint *(*function_type1)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *, struct mlib_bigint *),
    void (*function_type2)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *)
) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            struct mlib_bigint *bigintA = mlapi_get_bigint(U);

            mapi_push_arg(U, 1);
            struct mlib_bigint *bigintB = bigint_from(U);

            if (function_type1 != NULL) {
                function_type1(U, bigintA, bigintB, NULL);
            } else {
                function_type2(U, bigintA, bigintB);
            }
            maux_nb_return();
    maux_nb_end
}

static inline void raw_unary(
    morphine_coroutine_t U,
    struct mlib_bigint *(*function_type1)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *),
    void (*function_type2)(morphine_coroutine_t, struct mlib_bigint *)
) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            struct mlib_bigint *bigint = mlapi_get_bigint(U);

            if (function_type1 != NULL) {
                function_type1(U, bigint, NULL);
            } else {
                function_type2(U, bigint);
            }
            maux_nb_return();
    maux_nb_end
}

static inline void binary(
    morphine_coroutine_t U,
    struct mlib_bigint *(*function_type1)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *, struct mlib_bigint *),
    void (*function_type2)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *)
) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            maux_nb_operation("type", 1);
        maux_nb_state(1)
            mapi_push_result(U);
            struct mlib_bigint *bigintB;
            if (mapi_string_cstr_compare(U, BIGINT_WRAPPED_TYPE) == 0) {
                mapi_push_arg(U, 1);
                mapi_push_string(U, "instance");
                mapi_table_get(U);
                bigintB = mlapi_get_bigint(U);
            } else {
                mapi_push_arg(U, 1);
                bigintB = bigint_from(U);
            }

            mapi_push_arg(U, 0);
            mapi_push_string(U, "instance");
            mapi_table_get(U);
            struct mlib_bigint *bigintA = mlapi_get_bigint(U);

            if (function_type1 != NULL) {
                function_type1(U, bigintA, bigintB, NULL);
                bigint_metatable_wrap(U);
            } else {
                function_type2(U, bigintA, bigintB);
            }
            maux_nb_return();
    maux_nb_end
}

static inline void unary(
    morphine_coroutine_t U,
    struct mlib_bigint *(*function_type1)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *),
    void (*function_type2)(morphine_coroutine_t, struct mlib_bigint *)
) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_nb_operation("type", 1);
        maux_nb_state(1)
            mapi_push_result(U);
            struct mlib_bigint *bigintA;
            if (mapi_string_cstr_compare(U, BIGINT_WRAPPED_TYPE) == 0) {
                mapi_push_arg(U, 0);
                mapi_push_string(U, "instance");
                mapi_table_get(U);
                bigintA = mlapi_get_bigint(U);
            } else {
                mapi_push_arg(U, 0);
                bigintA = bigint_from(U);
            }

            if (function_type1 != NULL) {
                function_type1(U, bigintA, NULL);
                bigint_metatable_wrap(U);
            } else {
                function_type2(U, bigintA);
            }
            maux_nb_return();
    maux_nb_end
}

static void op_bigint_less(morphine_coroutine_t U, struct mlib_bigint *bigintA, struct mlib_bigint *bigintB) {
    mapi_push_boolean(U, mlapi_bigint_compare(bigintA, bigintB) < 0);
}

static void op_bigint_equal(
    morphine_coroutine_t U,
    struct mlib_bigint *bigintA,
    struct mlib_bigint *bigintB
) {
    mapi_push_boolean(U, mlapi_bigint_compare(bigintA, bigintB) == 0);
}

static void op_bigint_tostring(morphine_coroutine_t U, struct mlib_bigint *bigint) {
    mlapi_bigint_tostring(U, bigint);
}

static void op_bigint_compare(
    morphine_coroutine_t U,
    struct mlib_bigint *bigintA,
    struct mlib_bigint *bigintB
) {
    mapi_push_integer(U, mlapi_bigint_compare(bigintA, bigintB));
}

static void op_bigint_hash(morphine_coroutine_t U, struct mlib_bigint *bigint) {
    mapi_push_integer(U, mlapi_bigint_hash(U, bigint));
}

static void lib_bigint_add(morphine_coroutine_t U) {
    binary(U, mlapi_bigint_add, NULL);
}

static void lib_bigint_sub(morphine_coroutine_t U) {
    binary(U, mlapi_bigint_sub, NULL);
}

static void lib_bigint_mul(morphine_coroutine_t U) {
    binary(U, mlapi_bigint_mul, NULL);
}

static void lib_bigint_div(morphine_coroutine_t U) {
    binary(U, mlapi_bigint_div, NULL);
}

static void lib_bigint_mod(morphine_coroutine_t U) {
    binary(U, mlapi_bigint_mod, NULL);
}

static void lib_bigint_less(morphine_coroutine_t U) {
    binary(U, NULL, op_bigint_less);
}

static void lib_bigint_equal(morphine_coroutine_t U) {
    binary(U, NULL, op_bigint_equal);
}

static void lib_bigint_negate(morphine_coroutine_t U) {
    unary(U, mlapi_bigint_negate, NULL);
}

static void lib_bigint_tostring(morphine_coroutine_t U) {
    unary(U, NULL, op_bigint_tostring);
}

static void lib_bigint_compare(morphine_coroutine_t U) {
    binary(U, NULL, op_bigint_compare);
}

static void lib_bigint_hash(morphine_coroutine_t U) {
    unary(U, NULL, op_bigint_hash);
}

static void lib_raw_bigint_add(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_add, NULL);
}

static void lib_raw_bigint_sub(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_sub, NULL);
}

static void lib_raw_bigint_mul(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_mul, NULL);
}

static void lib_raw_bigint_div(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_div, NULL);
}

static void lib_raw_bigint_mod(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_mod, NULL);
}

static void lib_raw_bigint_less(morphine_coroutine_t U) {
    raw_binary(U, NULL, op_bigint_less);
}

static void lib_raw_bigint_equal(morphine_coroutine_t U) {
    raw_binary(U, NULL, op_bigint_equal);
}

static void lib_raw_bigint_negate(morphine_coroutine_t U) {
    raw_unary(U, mlapi_bigint_negate, NULL);
}

static void lib_raw_bigint_tostring(morphine_coroutine_t U) {
    raw_unary(U, NULL, op_bigint_tostring);
}

static void lib_raw_bigint_compare(morphine_coroutine_t U) {
    raw_binary(U, NULL, op_bigint_compare);
}

static void lib_raw_bigint_hash(morphine_coroutine_t U) {
    raw_unary(U, NULL, op_bigint_hash);
}

static void bigint_metatable_wrap(morphine_coroutine_t U) {
    maux_construct_element_t metatable_elements[] = {
        MAUX_CONSTRUCT_STRING(maux_metafield_name(U, MORPHINE_METAFIELD_TYPE), BIGINT_WRAPPED_TYPE),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_ADD), lib_bigint_add),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_SUB), lib_bigint_sub),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_MUL), lib_bigint_mul),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_DIV), lib_bigint_div),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_MOD), lib_bigint_mod),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_LESS), lib_bigint_less),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_EQUAL), lib_bigint_equal),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_NEGATE), lib_bigint_negate),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_TO_STRING), lib_bigint_tostring),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_COMPARE), lib_bigint_compare),
        MAUX_CONSTRUCT_FUNCTION(maux_metafield_name(U, MORPHINE_METAFIELD_HASH), lib_bigint_hash),
        MAUX_CONSTRUCT_NIL(maux_metafield_name(U, MORPHINE_METAFIELD_MASK)),
        MAUX_CONSTRUCT_END
    };

    mapi_push_table(U);
    mapi_push_string(U, "instance");
    mapi_peek(U, 2);
    mapi_table_set(U);

    maux_construct(U, metatable_elements);

    mapi_set_metatable(U);
    mapi_table_mode_mutable(U, false);
    mapi_table_mode_lock_metatable(U);
    mapi_table_mode_lock(U);

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

static void lib_bigint_create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bigint_from(U);
            maux_nb_return();
    maux_nb_end
}

static void lib_bigint_wrap(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bigint_from(U);
            bigint_metatable_wrap(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("create", lib_bigint_create),
    MAUX_CONSTRUCT_FUNCTION("wrap", lib_bigint_wrap),
    MAUX_CONSTRUCT_FUNCTION("add", lib_raw_bigint_add),
    MAUX_CONSTRUCT_FUNCTION("sub", lib_raw_bigint_sub),
    MAUX_CONSTRUCT_FUNCTION("mul", lib_raw_bigint_mul),
    MAUX_CONSTRUCT_FUNCTION("div", lib_raw_bigint_div),
    MAUX_CONSTRUCT_FUNCTION("mod", lib_raw_bigint_mod),
    MAUX_CONSTRUCT_FUNCTION("less", lib_raw_bigint_less),
    MAUX_CONSTRUCT_FUNCTION("equal", lib_raw_bigint_equal),
    MAUX_CONSTRUCT_FUNCTION("negate", lib_raw_bigint_negate),
    MAUX_CONSTRUCT_FUNCTION("tostring", lib_raw_bigint_tostring),
    MAUX_CONSTRUCT_FUNCTION("compare", lib_raw_bigint_compare),
    MAUX_CONSTRUCT_FUNCTION("hash", lib_raw_bigint_hash),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_bigint(void) {
    return (morphine_library_t) {
        .name = "bigint",
        .sharedkey = NULL,
        .init = library_init
    };
}
