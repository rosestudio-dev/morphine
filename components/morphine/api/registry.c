//
// Created by whyiskra on 07.01.24.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc//registry.h"
#include "morphine/object/coroutine/stack/access.h"

MORPHINE_API void mapi_registry_set_key(morphine_coroutine_t U) {
    struct value callable = stackI_peek(U, 1);
    struct value key = stackI_peek(U, 0);

    registryI_set_key(U->I, callable, key);

    stackI_pop(U, 1);
}

MORPHINE_API bool mapi_registry_get(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);

    bool has = false;
    *value = registryI_get(U, *value, &has);

    return has;
}

MORPHINE_API void mapi_registry_getoe(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);

    bool has = false;
    struct value result = registryI_get(U, *value, &has);

    if (has) {
        *value = result;
    } else {
        throwI_errorf(U->I, "Cannot get value from registry by %s", valueI_value2string(U->I, *value));
    }
}

MORPHINE_API void mapi_registry_set(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    registryI_set(U, key, value);

    stackI_pop(U, 2);
}

MORPHINE_API void mapi_registry_clear(morphine_coroutine_t U) {
    registryI_clear(U);
}
