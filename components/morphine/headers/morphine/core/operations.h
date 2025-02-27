//
// Created by whyiskra on 02.12.23.
//

#pragma once

#include "morphine/misc/metatable.h"
#include "morphine/object/table.h"
#include "morphine/object/reference.h"
#include "morphine/object/iterator.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/vector.h"
#include "morphine/core/convert.h"
#include "morphine/gc/safe.h"

typedef enum {
    NORMAL,
    CALLED,
    CALLED_COMPLETE
} op_result_t;

static inline op_result_t interpreter_fun_iterator(
    morphine_coroutine_t U,
    size_t callstate,
    struct value container,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, container, MORPHINE_METAFIELD_ITERATOR, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, container, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_object(iteratorI_create(U->I, container));
    return NORMAL;
}

static inline op_result_t interpreter_fun_iterator_init(
    morphine_coroutine_t U,
    size_t callstate,
    struct value iterator,
    struct value key_name,
    struct value value_name,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_iterator(iterator)) {
        iteratorI_init(U->I, valueI_as_iterator(iterator), key_name, value_name);
        return NORMAL;
    } else if (metatableI_builtin_test(U->I, iterator, MORPHINE_METAFIELD_ITERATOR_INIT, &mt_field)) {
        struct value new_args[] = { key_name, value_name };
        callstackI_continue(U, callstate);
        callstackI_call_unsafe(U, mt_field, iterator, new_args, 2, pop_size);
        return CALLED;
    }

    throwI_error(U->I, "cannot init iterator");
}

static inline op_result_t interpreter_fun_iterator_has(
    morphine_coroutine_t U,
    size_t callstate,
    struct value iterator,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_iterator(iterator)) {
        bool has = iteratorI_has(U->I, valueI_as_iterator(iterator));
        (*result) = valueI_boolean(has);
        return NORMAL;
    } else if (metatableI_builtin_test(U->I, iterator, MORPHINE_METAFIELD_ITERATOR_HAS, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, iterator, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "cannot check next value of iterator");
}

static inline op_result_t interpreter_fun_iterator_next(
    morphine_coroutine_t U,
    size_t callstate,
    struct value iterator,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (likely(valueI_is_iterator(iterator))) {
        (*result) = valueI_object(iteratorI_next_table(U->I, valueI_as_iterator(iterator)));
        return NORMAL;
    } else if (metatableI_builtin_test(U->I, iterator, MORPHINE_METAFIELD_ITERATOR_NEXT, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, iterator, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "cannot get next value of iterator");
}

static inline op_result_t interpreter_fun_get(
    morphine_coroutine_t U,
    size_t callstate,
    struct value container,
    struct value key,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_vector(container)) {
        ml_size index = valueI_integer2index(U->I, valueI_as_integer_or_error(U->I, key));
        (*result) = vectorI_get(U->I, valueI_as_vector(container), index);
        return NORMAL;
    } else if (valueI_is_string(container)) {
        ml_size index = valueI_integer2index(U->I, valueI_as_integer_or_error(U->I, key));
        (*result) = valueI_object(stringI_get(U->I, valueI_as_string(container), index));
        return NORMAL;
    } else if (metatableI_builtin_test(U->I, container, MORPHINE_METAFIELD_GET, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { key };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, container, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    } else if (valueI_is_table(container)) {
        (*result) = tableI_get(U->I, valueI_as_table(container), key, NULL);
        return NORMAL;
    }

    throwI_error(U->I, "get supports only table, vector or string");
}

static inline op_result_t interpreter_fun_set(
    morphine_coroutine_t U,
    size_t callstate,
    struct value container,
    struct value key,
    struct value value,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (valueI_is_vector(container)) {
        ml_size index = valueI_integer2index(U->I, valueI_as_integer_or_error(U->I, key));
        vectorI_set(U->I, valueI_as_vector(container), index, value);
        return NORMAL;
    } else if (metatableI_builtin_test(U->I, container, MORPHINE_METAFIELD_SET, &mt_field)) {
        struct value new_args[] = { key, value };
        callstackI_continue(U, callstate);
        callstackI_call_unsafe(U, mt_field, container, new_args, 2, pop_size);
        return CALLED;
    } else if (valueI_is_table(container)) {
        tableI_set(U->I, valueI_as_table_or_error(U->I, container), key, value);
        return NORMAL;
    }

    throwI_error(U->I, "set supports only table or vector");
}

static inline op_result_t interpreter_fun_add(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_ADD, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "add supports only integer or decimal");
}

static inline op_result_t interpreter_fun_sub(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_SUB, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "sub supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mul(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_MUL, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "mul supports only integer or decimal");
}

static inline op_result_t interpreter_fun_div(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        ml_integer b_value = convertI_to_integer(U->I, b);
        if (unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) / b_value);

        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        ml_decimal b_value = convertI_to_decimal(U->I, b);
        if (unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_decimal(valueI_as_decimal(a) / b_value);

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_DIV, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "div supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mod(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_integer(a)) {
        ml_integer b_value = convertI_to_integer(U->I, b);
        if (unlikely(b_value == 0)) {
            throwI_error(U->I, "attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) % b_value);

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_MOD, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "mod supports only integer");
}

static inline op_result_t interpreter_fun_equal(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_EQUAL, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_boolean(valueI_equal(U->I, a, b));
    return NORMAL;
}

static inline op_result_t interpreter_fun_less(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_LESS, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "less supports only integer or decimal");
}

static inline op_result_t interpreter_fun_and(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_AND, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    if (convertI_to_boolean(a)) {
        (*result) = b;
    } else {
        (*result) = a;
    }

    return NORMAL;
}

static inline op_result_t interpreter_fun_or(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_OR, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    if (convertI_to_boolean(a)) {
        (*result) = a;
    } else {
        (*result) = b;
    }

    return NORMAL;
}

static inline op_result_t interpreter_fun_concat(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_CONCAT, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            struct value new_args[] = { b };
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, new_args, 1, pop_size);
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
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_TYPE, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
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
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
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
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_NEGATE, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "negate supports only integer or decimal");
}

static inline op_result_t interpreter_fun_not(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_NOT, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_boolean(!convertI_to_boolean(a));
    return NORMAL;
}

static inline op_result_t interpreter_fun_ref(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_REF, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    (*result) = valueI_object(referenceI_create(U->I, a));
    return NORMAL;
}

static inline op_result_t interpreter_fun_deref(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_reference(a)) {
        (*result) = *referenceI_get(U->I, valueI_as_reference_or_error(U->I, a));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_DEREF, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "deref supports only reference");
}

static inline op_result_t interpreter_fun_length(
    morphine_coroutine_t U,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (unlikely(need_return && (callstackI_state(U) == callstate))) {
        (*result) = callstackI_result(U);
        return CALLED_COMPLETE;
    }

    if (valueI_is_string(a)) {
        (*result) = valueI_size(valueI_as_string(a)->size);
        return NORMAL;
    }

    if (valueI_is_table(a)) {
        (*result) = valueI_size(tableI_size(U->I, valueI_as_table(a)));
        return NORMAL;
    }

    if (valueI_is_vector(a)) {
        (*result) = valueI_size(vectorI_size(U->I, valueI_as_vector(a)));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_builtin_test(U->I, a, MORPHINE_METAFIELD_LENGTH, &mt_field)) {
        if (valueI_is_callable(mt_field)) {
            callstackI_continue(U, callstate);
            callstackI_call_unsafe(U, mt_field, a, NULL, 0, pop_size);
            return CALLED;
        }

        (*result) = mt_field;
        return NORMAL;
    }

    throwI_error(U->I, "length supports only string, table or vector");
}
