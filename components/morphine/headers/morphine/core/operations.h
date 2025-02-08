//
// Created by whyiskra on 02.12.23.
//

#pragma once

#include "morphine/core/convert.h"
#include "morphine/core/metatable.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/table.h"
#include "morphine/object/vector.h"
#include "morphine/utils/array_size.h"

typedef enum {
    NORMAL,
    CALLED,
    CALLED_COMPLETE
} op_result_t;

static inline op_result_t interpreter_fun_get(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value container,
    struct value key,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_vector(container)) {
        ml_size index = valueI_as_index_or_error(U->I, key);
        (*result) = vectorI_get(U->I, valueI_as_vector(container), index);
        return NORMAL;
    } else if (valueI_is_string(container)) {
        ml_size index = valueI_as_index_or_error(U->I, key);
        (*result) = valueI_object(stringI_get(U->I, valueI_as_string(container), index));
        return NORMAL;
    } else if (metatableI_test(U->I, container, MTYPE_METAFIELD_GET, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { container, key };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    } else if (valueI_is_table(container)) {
        (*result) = tableI_get(valueI_as_table(container), key, NULL);
        return NORMAL;
    }

    throwI_error(U->I, "get supports only table, vector or string");
}

static inline op_result_t interpreter_fun_set(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value container,
    struct value key,
    struct value value,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_vector(container)) {
        ml_size index = valueI_as_index_or_error(U->I, key);
        vectorI_set(U->I, valueI_as_vector(container), index, value);
        return NORMAL;
    } else if (metatableI_test(U->I, container, MTYPE_METAFIELD_SET, &mt_field)) {
        struct value new_args[] = { container, key, value };
        callstackI_continue(U, callstate);
        callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
        return CALLED;
    } else if (valueI_is_table(container)) {
        tableI_set(U->I, valueI_as_table_or_error(U->I, container), key, value);
        return NORMAL;
    }

    throwI_error(U->I, "set supports only table or vector");
}

static inline op_result_t interpreter_fun_add(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        (*result) = valueI_integer(valueI_as_integer(a) + convertI_to_integer(U->I, b));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) + convertI_to_decimal(U->I, b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_ADD, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "add supports only integer or decimal");
}

static inline op_result_t interpreter_fun_sub(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        (*result) = valueI_integer(valueI_as_integer(a) - convertI_to_integer(U->I, b));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) - convertI_to_decimal(U->I, b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_SUB, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "sub supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mul(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        (*result) = valueI_integer(valueI_as_integer(a) * convertI_to_integer(U->I, b));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) * convertI_to_decimal(U->I, b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_MUL, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "mul supports only integer or decimal");
}

static inline op_result_t interpreter_fun_div(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        ml_integer b_value = convertI_to_integer(U->I, b);
        if (mm_unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) / b_value);

        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        ml_decimal b_value = convertI_to_decimal(U->I, b);
        if (mm_unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_decimal(valueI_as_decimal(a) / b_value);

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_DIV, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "div supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mod(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        ml_integer b_value = convertI_to_integer(U->I, b);
        if (mm_unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) % b_value);

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_MOD, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "mod supports only integer");
}

static inline op_result_t interpreter_fun_equal(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_EQUAL, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_boolean(valueI_compare(a, b) == 0);
    return NORMAL;
}

static inline op_result_t interpreter_fun_less(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        (*result) = valueI_boolean(valueI_as_integer(a) < convertI_to_integer(U->I, b));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_boolean(valueI_as_decimal(a) < convertI_to_decimal(U->I, b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_LESS, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "less supports only integer or decimal");
}

static inline op_result_t interpreter_fun_and(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_AND, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    if (valueI_tobool(a)) {
        (*result) = b;
    } else {
        (*result) = a;
    }

    return NORMAL;
}

static inline op_result_t interpreter_fun_or(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_OR, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    if (valueI_tobool(a)) {
        (*result) = a;
    } else {
        (*result) = b;
    }

    return NORMAL;
}

static inline op_result_t interpreter_fun_concat(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_string(a) && valueI_is_string(b)) {
        struct string *a_str = valueI_as_string(a);
        struct string *b_str = valueI_as_string(b);
        (*result) = valueI_object(stringI_concat(U->I, a_str, b_str));

        return NORMAL;
    }

    if (valueI_is_vector(a) && valueI_is_vector(b)) {
        struct vector *a_vec = valueI_as_vector(a);
        struct vector *b_vec = valueI_as_vector(b);
        (*result) = valueI_object(vectorI_concat(U->I, a_vec, b_vec));

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_CONCAT, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    } else if (valueI_is_table(a) && valueI_is_table(b)) {
        struct table *a_table = valueI_as_table(a);
        struct table *b_table = valueI_as_table(b);
        (*result) = valueI_object(tableI_concat(U->I, a_table, b_table));

        return NORMAL;
    }

    throwI_error(U->I, "concat supports only string, vector or table");
}

static inline op_result_t interpreter_fun_type(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_TYPE, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_object(stringI_create(U->I, valueI_type(U->I, a, false)));
    return NORMAL;
}

static inline op_result_t interpreter_fun_negative(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        (*result) = valueI_integer(-valueI_as_integer(a));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_decimal(-valueI_as_decimal(a));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_NEGATE, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "negate supports only integer or decimal");
}

static inline op_result_t interpreter_fun_not(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_NOT, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_boolean(!valueI_tobool(a));
    return NORMAL;
}

static inline op_result_t interpreter_fun_length(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_string(a)) {
        (*result) = valueI_size(valueI_as_string(a)->size);
        return NORMAL;
    }

    if (valueI_is_table(a)) {
        (*result) = valueI_size(tableI_size(valueI_as_table(a)));
        return NORMAL;
    }

    if (valueI_is_vector(a)) {
        (*result) = valueI_size(vectorI_size(valueI_as_vector(a)));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_LENGTH, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "length supports only string, table or vector");
}

static inline op_result_t interpreter_fun_tostr(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_TO_STRING, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_object(convertI_to_string(U->I, a));
    return NORMAL;
}

static inline op_result_t interpreter_fun_compare(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value b,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_COMPARE, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a, b };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_integer(valueI_compare(a, b));
    return NORMAL;
}


static inline op_result_t interpreter_fun_hash(
    morphine_coroutine_t U,
    ml_size callstate,
    struct value a,
    struct value *result,
    ml_size pop_size,
    bool need_return
) {
    if (mm_unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(U->I, a, MTYPE_METAFIELD_HASH, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { a };
            callstackI_continue(U, callstate);
            callstackI_call(U, &mt_field, new_args, array_size(new_args), pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_integer(valueI_hash(a));
    return NORMAL;
}
