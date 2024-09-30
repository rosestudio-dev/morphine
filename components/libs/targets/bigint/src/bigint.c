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
    struct mlib_bigint *(*function_type1)(morphine_coroutine_t, struct mlib_bigint *, struct mlib_bigint *, struct mlib_bigint *)
) {
    maux_nb_function(U)
        maux_nb_init
            struct mlib_bigint *bigintA;
            struct mlib_bigint *bigintB;
            struct mlib_bigint *result = NULL;
            if (mapi_args(U) == 3) {
                mapi_push_arg(U, 0);
                bigintA = mlapi_get_bigint(U);

                mapi_push_arg(U, 1);
                bigintB = bigint_from(U);

                mapi_push_arg(U, 2);
                result = mlapi_get_bigint(U);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 0);
                bigintA = mlapi_get_bigint(U);

                mapi_push_arg(U, 1);
                bigintB = bigint_from(U);
            }

            function_type1(U, bigintA, bigintB, result);
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
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
        maux_nb_operation(1, "type")
            struct mlib_bigint *bigintB;
            if (mapi_string_cstr_compare(U, BIGINT_WRAPPED_TYPE) == 0) {
                mapi_push_arg(U, 0);
                mapi_push_string(U, "instance");
                mapi_table_get(U);
                bigintB = mlapi_get_bigint(U);
            } else {
                mapi_push_arg(U, 0);
                bigintB = bigint_from(U);
            }

            mapi_push_self(U);
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

static void op_bigint_less(morphine_coroutine_t U, struct mlib_bigint *bigintA, struct mlib_bigint *bigintB) {
    mapi_push_boolean(U, mlapi_bigint_compare(bigintA, bigintB) < 0);
}

static void lib_bigint_less(morphine_coroutine_t U) {
    binary(U, NULL, op_bigint_less);
}

static void op_bigint_equal(morphine_coroutine_t U, struct mlib_bigint *bigintA, struct mlib_bigint *bigintB) {
    mapi_push_boolean(U, mlapi_bigint_compare(bigintA, bigintB) == 0);
}

static void lib_bigint_equal(morphine_coroutine_t U) {
    binary(U, NULL, op_bigint_equal);
}

static void lib_bigint_negate(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            mapi_push_self(U);
            mapi_push_string(U, "instance");
            mapi_table_get(U);
            struct mlib_bigint *bigint = mlapi_get_bigint(U);

            mlapi_bigint_negate(U, bigint, false);
            maux_nb_return();
    maux_nb_end
}

static void lib_bigint_tostring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            mapi_push_self(U);
            mapi_push_string(U, "instance");
            mapi_table_get(U);
            struct mlib_bigint *bigint = mlapi_get_bigint(U);

            mlapi_bigint_tostring(U, bigint);
            maux_nb_return();
    maux_nb_end
}

static void lib_raw_bigint_add(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_add);
}

static void lib_raw_bigint_sub(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_sub);
}

static void lib_raw_bigint_mul(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_mul);
}

static void lib_raw_bigint_div(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_div);
}

static void lib_raw_bigint_mod(morphine_coroutine_t U) {
    raw_binary(U, mlapi_bigint_mod);
}

static void lib_raw_bigint_compare(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            struct mlib_bigint *bigintA = mlapi_get_bigint(U);

            mapi_push_arg(U, 1);
            struct mlib_bigint *bigintB = bigint_from(U);

            mapi_push_integer(U, mlapi_bigint_compare(bigintA, bigintB));
            maux_nb_return();
    maux_nb_end
}

static void lib_raw_bigint_negate(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            struct mlib_bigint *bigint = mlapi_get_bigint(U);
            mlapi_bigint_negate(U, bigint, true);
            maux_nb_return();
    maux_nb_end
}

static void lib_raw_bigint_tostring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            struct mlib_bigint *bigint = mlapi_get_bigint(U);
            mlapi_bigint_tostring(U, bigint);
            maux_nb_return();
    maux_nb_end
}

static void bigint_metatable_wrap(morphine_coroutine_t U) {
    mapi_push_table(U);
    mapi_push_string(U, "instance");
    mapi_peek(U, 2);
    mapi_table_set(U);

    mapi_push_table(U);

    mapi_push_string(U, "_mf_type");
    mapi_push_string(U, BIGINT_WRAPPED_TYPE);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_to_string");
    maux_push_native(U, "bigint.tostring", lib_bigint_tostring);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_add");
    maux_push_native(U, "bigint.add", lib_bigint_add);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_sub");
    maux_push_native(U, "bigint.sub", lib_bigint_sub);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_mul");
    maux_push_native(U, "bigint.mul", lib_bigint_mul);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_div");
    maux_push_native(U, "bigint.div", lib_bigint_div);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_mod");
    maux_push_native(U, "bigint.mod", lib_bigint_mod);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_less");
    maux_push_native(U, "bigint.less", lib_bigint_less);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_equal");
    maux_push_native(U, "bigint.equal", lib_bigint_equal);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_negate");
    maux_push_native(U, "bigint.negate", lib_bigint_negate);
    mapi_table_set(U);

    mapi_push_string(U, "_mf_mask");
    mapi_push_nil(U);
    mapi_table_set(U);

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

static morphine_library_function_t functions[] = {
    { "create",   lib_bigint_create },
    { "wrap",     lib_bigint_wrap },
    { "tostring", lib_raw_bigint_tostring },
    { "add",      lib_raw_bigint_add },
    { "sub",      lib_raw_bigint_sub },
    { "mul",      lib_raw_bigint_mul },
    { "div",      lib_raw_bigint_div },
    { "mod",      lib_raw_bigint_mod },
    { "negate",   lib_raw_bigint_negate },
    { "compare",  lib_raw_bigint_compare },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "bigint",
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mllib_bigint(void) {
    return &library;
}
