//
// Created by whyiskra on 07.01.24.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/misc//registry.h"
#include "morphine/stack/access.h"

MORPHINE_API void mapi_registry_set_key(morphine_state_t S) {
    struct value callable = stackI_peek(S, 1);
    struct value key = stackI_peek(S, 0);

    registryI_set_key(S->I, callable, key);

    stackI_pop(S, 1);
}

MORPHINE_API bool mapi_registry_get(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);

    bool has = false;
    *value = registryI_get(S, *value, &has);

    return has;
}

MORPHINE_API void mapi_registry_getoe(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);

    bool has = false;
    struct value result = registryI_get(S, *value, &has);

    if (has) {
        *value = result;
    } else {
        throwI_errorf(S->I, "Cannot get value from registry by %s", valueI_value2string(S->I, *value));
    }
}

MORPHINE_API void mapi_registry_set(morphine_state_t S) {
    struct value key = stackI_peek(S, 1);
    struct value value = stackI_peek(S, 0);

    registryI_set(S, key, value);

    stackI_pop(S, 2);
}

MORPHINE_API void mapi_registry_clear(morphine_state_t S) {
    registryI_clear(S);
}
