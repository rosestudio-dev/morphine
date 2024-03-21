//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_table(morphine_state_t S, size_t caps) {
    stackI_push(S, valueI_object(tableI_create(S->I, caps)));
}

MORPHINE_API void mapi_table_set(morphine_state_t S) {
    struct value table = stackI_peek(S, 2);
    struct value key = stackI_peek(S, 1);
    struct value value = stackI_peek(S, 0);

    tableI_set(S->I, valueI_as_table_or_error(S, table), key, value);

    stackI_pop(S, 2);
}

MORPHINE_API bool mapi_table_get(morphine_state_t S) {
    struct value table = stackI_peek(S, 1);
    struct value *value = stackI_vector(S, 0, 1);

    bool has = false;
    *value = tableI_get(S->I, valueI_as_table_or_error(S, table), *value, &has);

    return has;
}

MORPHINE_API void mapi_table_getoe(morphine_state_t S) {
    struct value table = stackI_peek(S, 1);
    struct value *value = stackI_vector(S, 0, 1);

    bool has = false;
    struct value result = tableI_get(S->I, valueI_as_table_or_error(S, table), *value, &has);

    if (has) {
        *value = result;
    } else {
        throwI_errorf(S, "Cannot get value from table by %s", valueI_value2string(S->I, *value));
    }
}

MORPHINE_API size_t mapi_table_len(morphine_state_t S) {
    return tableI_size(S->I, valueI_as_table_or_error(S, stackI_peek(S, 0)));
}
