//
// Created by whyiskra on 02.12.23.
//

#pragma once

#include "metatable.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/reference.h"
#include "morphine/utils/unused.h"
#include "call.h"

typedef enum {
    NORMAL,
    CALLED,
    CALLED_COMPLETE
} op_result_t;

static inline op_result_t interpreter_fun_get(
    morphine_state_t S,
    size_t callstate,
    struct value table,
    struct value key,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (morphinem_likely(valueI_is_table(table))) {
        bool has = false;
        (*result) = tableI_get(S->I, valueI_as_table(table), key, &has);

        if (!has && metatableI_test(S->I, table, MF_GET, &mt_field)) {
            struct value new_args[] = {key};
            callI_continue(S, callstate);
            callI_do(S, mt_field, table, 1, new_args, pop_size);
            return CALLED;
        }

        return NORMAL;
    } else if (metatableI_test(S->I, table, MF_GET, &mt_field)) {
        struct value new_args[] = {key};
        callI_continue(S, callstate);
        callI_do(S, mt_field, table, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Get supports only table");
}

static inline op_result_t interpreter_fun_set(
    morphine_state_t S,
    size_t callstate,
    struct value table,
    struct value key,
    struct value value,
    size_t pop_size,
    bool need_return
) {
    morphinem_unused(need_return);

    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(S->I, table, MF_SET, &mt_field)) {
        struct value new_args[] = {key, value};
        callI_continue(S, callstate);
        callI_do(S, mt_field, table, 1, new_args, pop_size);
        return CALLED;
    }

    tableI_set(S->I, valueI_as_table_or_error(S, table), key, value);
    return NORMAL;
}

static inline op_result_t interpreter_fun_add(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        (*result) = valueI_integer(valueI_as_integer(a) + valueI_as_integer(b));
        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) + valueI_as_decimal(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_ADD, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Add supports only integer or decimal");
}

static inline op_result_t interpreter_fun_sub(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        (*result) = valueI_integer(valueI_as_integer(a) - valueI_as_integer(b));
        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) - valueI_as_decimal(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_SUB, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Sub supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mul(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        (*result) = valueI_integer(valueI_as_integer(a) * valueI_as_integer(b));
        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        (*result) = valueI_decimal(valueI_as_decimal(a) * valueI_as_decimal(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_MUL, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Mul supports only integer or decimal");
}

static inline op_result_t interpreter_fun_div(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        if (morphinem_unlikely(valueI_as_integer(b) == 0)) {
            throwI_message_error(S, "Attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) / valueI_as_integer(b));

        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        if (morphinem_unlikely(valueI_as_decimal(b) == 0)) {
            throwI_message_error(S, "Attempt to divide by zero");
        }

        (*result) = valueI_decimal(valueI_as_decimal(a) / valueI_as_decimal(b));

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_DIV, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Div supports only integer or decimal");
}

static inline op_result_t interpreter_fun_mod(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        if (morphinem_unlikely(valueI_as_integer(b) == 0)) {
            throwI_message_error(S, "Attempt to divide by zero");
        }

        (*result) = valueI_integer(valueI_as_integer(a) % valueI_as_integer(b));

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_MOD, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Mod supports only integer");
}

static inline op_result_t interpreter_fun_equal(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_EQUAL, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    (*result) = valueI_boolean(valueI_equal(S->I, a, b));
    return NORMAL;
}

static inline op_result_t interpreter_fun_less(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        (*result) = valueI_boolean(valueI_as_integer(a) < valueI_as_integer(b));
        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        (*result) = valueI_boolean(valueI_as_decimal(a) < valueI_as_decimal(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_LESS, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Less supports only integer or decimal");
}

static inline op_result_t interpreter_fun_less_equal(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a) && valueI_is_integer(b))) {
        (*result) = valueI_boolean(valueI_as_integer(a) <= valueI_as_integer(b));
        return NORMAL;
    }

    if (valueI_is_decimal(a) && valueI_is_decimal(b)) {
        (*result) = valueI_boolean(valueI_as_decimal(a) <= valueI_as_decimal(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_LESS_EQUAL, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Less equal supports only integer or decimal");
}

static inline op_result_t interpreter_fun_and(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_boolean(a) && valueI_is_boolean(b))) {
        (*result) = valueI_boolean(valueI_as_boolean(a) && valueI_as_boolean(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_AND, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "And supports only boolean");
}

static inline op_result_t interpreter_fun_or(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_boolean(a) && valueI_is_boolean(b))) {
        (*result) = valueI_boolean(valueI_as_boolean(a) || valueI_as_boolean(b));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_OR, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Or supports only boolean");
}

static inline op_result_t interpreter_fun_concat(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value b,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_string(a) && valueI_is_string(b))) {
        struct string *a_str = valueI_as_string(a);
        struct string *b_str = valueI_as_string(b);
        (*result) = valueI_object(stringI_createf(S->I, "%s%s", a_str->chars, b_str->chars));

        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_CONCAT, &mt_field)) {
        struct value new_args[] = {b};
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 1, new_args, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Concat supports only string");
}

static inline op_result_t interpreter_fun_type(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_TYPE, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    (*result) = valueI_object(stringI_create(S->I, valueI_type2string(S->I, a.type)));
    return NORMAL;
}

static inline op_result_t interpreter_fun_negative(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_integer(a))) {
        (*result) = valueI_integer(-valueI_as_integer(a));
        return NORMAL;
    }

    if (valueI_is_decimal(a)) {
        (*result) = valueI_decimal(-valueI_as_decimal(a));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_NEGATE, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Negate supports only integer or decimal");
}

static inline op_result_t interpreter_fun_not(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_boolean(a))) {
        (*result) = valueI_boolean(!valueI_as_boolean(a));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_NOT, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Not supports only boolean");
}

static inline op_result_t interpreter_fun_ref(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_REF, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    (*result) = valueI_object(referenceI_create(S->I, a));
    return NORMAL;
}

static inline op_result_t interpreter_fun_deref(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (morphinem_likely(valueI_is_reference(a))) {
        (*result) = *referenceI_get(S->I, valueI_as_reference_or_error(S, a));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_DEREF, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Deref supports only reference");
}

static inline op_result_t interpreter_fun_length(
    morphine_state_t S,
    size_t callstate,
    struct value a,
    struct value *result,
    size_t pop_size,
    bool need_return
) {
    if (morphinem_unlikely(need_return && (callI_callstate(S) == callstate))) {
        (*result) = callI_result(S);
        return CALLED_COMPLETE;
    }

    if (valueI_is_string(a)) {
        (*result) = valueI_size2integer(S, valueI_as_string(a)->size);
        return NORMAL;
    }

    if (valueI_is_table(a)) {
        (*result) = valueI_size2integer(S, tableI_size(S->I, valueI_as_table_or_error(S, a)));
        return NORMAL;
    }

    struct value mt_field;
    if (metatableI_test(S->I, a, MF_LENGTH, &mt_field)) {
        callI_continue(S, callstate);
        callI_do(S, mt_field, a, 0, NULL, pop_size);
        return CALLED;
    }

    throwI_message_error(S, "Length supports only string or table");
}
