//
// Created by why-iskra on 22.07.2024.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include <string.h>

MORPHINE_API const char *mapi_type(morphine_coroutine_t U) {
    return valueI_type(U->I, stackI_peek(U, 0), false);
}

MORPHINE_API bool mapi_is(morphine_coroutine_t U, const char *type) {
    if (strcmp(type, "callable") == 0) {
        return mapi_is_callable(U);
    }

    if (strcmp(type, "meta") == 0) {
        return mapi_is_metatype(U);
    }

    if (strcmp(type, "iterable") == 0) {
        return mapi_is_iterable(U);
    }

    return mapi_is_type(U, type);
}

MORPHINE_API bool mapi_is_type(morphine_coroutine_t U, const char *type) {
    return strcmp(valueI_type(U->I, stackI_peek(U, 0), false), type) == 0;
}

MORPHINE_API bool mapi_is_callable(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_callable(value);
}

MORPHINE_API bool mapi_is_metatype(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_metatype(value);
}

MORPHINE_API bool mapi_is_iterable(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_iterable(value);
}
