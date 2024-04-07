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

MORPHINE_API void mapi_table_set(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 2);
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    tableI_set(U->I, valueI_as_table_or_error(U->I, table), key, value);

    stackI_pop(U, 2);
}

MORPHINE_API bool mapi_table_get(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 1);
    struct value *value = stackI_vector(U, 0, 1);

    bool has = false;
    *value = tableI_get(U->I, valueI_as_table_or_error(U->I, table), *value, &has);

    return has;
}

MORPHINE_API void mapi_table_getoe(morphine_coroutine_t U) {
    struct value table = stackI_peek(U, 1);
    struct value *value = stackI_vector(U, 0, 1);

    bool has = false;
    struct value result = tableI_get(U->I, valueI_as_table_or_error(U->I, table), *value, &has);

    if (has) {
        *value = result;
    } else {
        throwI_errorf(U->I, "Cannot get value from table by %s", valueI_value2string(U->I, *value));
    }
}

MORPHINE_API ml_size mapi_table_len(morphine_coroutine_t U) {
    return tableI_size(U->I, valueI_as_table_or_error(U->I, stackI_peek(U, 0)));
}
