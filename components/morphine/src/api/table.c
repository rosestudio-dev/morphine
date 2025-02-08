//
// Created by whyiskra on 30.12.23.
//

#include "morphine/object/table.h"
#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"

static inline void push_pair(morphine_coroutine_t U, struct pair pair, bool replace) {
    gcI_safe_enter(U->I);
    gcI_safe(U->I, pair.key);
    gcI_safe(U->I, pair.value);

    if (replace) {
        stackI_replace(U, 0, pair.key);
    } else {
        stackI_push(U, pair.key);
    }
    stackI_push(U, pair.value);

    gcI_safe_exit(U->I);
}

MORPHINE_API void mapi_push_table(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(tableI_create(U->I)));
}

MORPHINE_API void mapi_table_copy(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(tableI_copy(U->I, table)));
}

MORPHINE_API void mapi_table_concat(morphine_coroutine_t U) {
    struct table *a = valueI_as_table_or_error(U->I, stackI_peek(U, 1));
    struct table *b = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    struct table *result = tableI_concat(U->I, a, b);
    stackI_replace(U, 1, valueI_object(result));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_table_clear(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_clear(U->I, table);
}

MORPHINE_API void mapi_table_set(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 2);
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    tableI_set(U->I, valueI_as_table_or_error(U->I, table), key, value);
    stackI_pop(U, 2);
}

MORPHINE_API bool mapi_table_has(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    return tableI_has(table, value);
}

MORPHINE_API bool mapi_table_get(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_get(valueI_as_table_or_error(U->I, table), value, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API bool mapi_table_remove(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_remove(U->I, valueI_as_table_or_error(U->I, table), value, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API void mapi_table_idx_set(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    tableI_idx_set(U->I, valueI_as_table_or_error(U->I, table), index, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_table_idx_get(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);
    struct pair result = tableI_idx_get(U->I, valueI_as_table_or_error(U->I, table), index);
    push_pair(U, result, false);
}

MORPHINE_API void mapi_table_idx_remove(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);
    struct pair result = tableI_idx_remove(U->I, valueI_as_table_or_error(U->I, table), index);
    push_pair(U, result, false);
}

MORPHINE_API ml_size mapi_table_len(morphine_coroutine_t U) {
    return tableI_size(valueI_as_table_or_error(U->I, stackI_peek(U, 0)));
}

MORPHINE_API bool mapi_table_first(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));

    bool has = false;
    struct pair pair = tableI_first(table, &has);
    push_pair(U, pair, false);

    return has;
}

MORPHINE_API bool mapi_table_next(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 1));
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct pair pair = tableI_next(table, key, &has);
    push_pair(U, pair, true);

    return has;
}
