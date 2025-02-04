//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"

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

MORPHINE_API void mapi_table_mode_mutable(morphine_coroutine_t U, bool is_mutable) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_mode_mutable(U->I, table, is_mutable);
}

MORPHINE_API void mapi_table_mode_fixed(morphine_coroutine_t U, bool is_fixed) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_mode_fixed(U->I, table, is_fixed);
}

MORPHINE_API void mapi_table_mode_accessible(morphine_coroutine_t U, bool is_accessible) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_mode_accessible(U->I, table, is_accessible);
}

MORPHINE_API void mapi_table_lock_metatable(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_lock_metatable(U->I, table);
}

MORPHINE_API void mapi_table_lock_mode(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    tableI_lock_mode(U->I, table);
}

MORPHINE_API bool mapi_table_mode_is_mutable(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    return table->mode.mutable;
}

MORPHINE_API bool mapi_table_mode_is_fixed(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    return table->mode.fixed;
}

MORPHINE_API bool mapi_table_mode_is_accessible(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    return table->mode.accessible;
}

MORPHINE_API bool mapi_table_metatable_is_locked(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    return table->lock.metatable;
}

MORPHINE_API bool mapi_table_mode_is_locked(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    return table->lock.mode;
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

    return tableI_has(U->I, table, value);
}

MORPHINE_API bool mapi_table_get(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_get(U->I, valueI_as_table_or_error(U->I, table), value, &has);
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

MORPHINE_API bool mapi_table_idx_get(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_idx_get(U->I, valueI_as_table_or_error(U->I, table), index, &has).value;
    stackI_push(U, result);

    return has;
}

MORPHINE_API bool mapi_table_idx_key(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_idx_get(U->I, valueI_as_table_or_error(U->I, table), index, &has).key;
    stackI_push(U, result);

    return has;
}

MORPHINE_API void mapi_table_idx_getoe(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_idx_get(U->I, valueI_as_table_or_error(U->I, table), index, &has).value;

    if (has) {
        stackI_push(U, result);
    } else {
        throwI_errorf(U->I, "cannot get value from table by index %"MLIMIT_SIZE_PR, index);
    }
}

MORPHINE_API void mapi_table_idx_keyoe(morphine_coroutine_t U, ml_size index) {
    struct value table = stackI_peek(U, 0);

    bool has = false;
    struct value result = tableI_idx_get(U->I, valueI_as_table_or_error(U->I, table), index, &has).key;

    if (has) {
        stackI_push(U, result);
    } else {
        throwI_errorf(U->I, "cannot get key from table by index %"MLIMIT_SIZE_PR, index);
    }
}

MORPHINE_API void mapi_table_getoe(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);

    if (tableI_has(U->I, table, value)) {
        struct value result = tableI_get(U->I, table, value, NULL);
        stackI_replace(U, 0, result);
    } else {
        throwI_errorf(U->I, "cannot get value from table by key");
    }
}

MORPHINE_API void mapi_table_removeoe(morphine_coroutine_t U) {
    struct table *table = valueI_as_table_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);

    if (tableI_has(U->I, table, value)) {
        struct value result = tableI_remove(U->I, table, value, NULL);
        stackI_replace(U, 0, result);
    } else {
        throwI_errorf(U->I, "cannot remove from table by key");
    }
}

MORPHINE_API ml_size mapi_table_len(morphine_coroutine_t U) {
    return tableI_size(U->I, valueI_as_table_or_error(U->I, stackI_peek(U, 0)));
}
