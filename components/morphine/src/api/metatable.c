//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/core/metatable.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_set_metatable(morphine_coroutine_t U) {
    struct value source = stackI_peek(U, 1);
    struct value metatable = stackI_peek(U, 0);

    if (valueI_is_nil(metatable)) {
        metatableI_set(U->I, source, NULL);
    } else {
        metatableI_set(U->I, source, valueI_as_table_or_error(U->I, metatable));
    }

    stackI_pop(U, 1);
}

MORPHINE_API void mapi_get_metatable(morphine_coroutine_t U) {
    struct value source = stackI_peek(U, 0);
    struct value result = metatableI_get(U->I, source);
    stackI_push(U, result);
}
