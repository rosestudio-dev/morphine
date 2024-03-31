//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/core/metatable.h"
#include "morphine/stack/access.h"

MORPHINE_API bool mapi_metatable_test(morphine_state_t S, const char *field) {
    struct value source = stackI_peek(S, 0);

    struct value mt;
    bool result = metatableI_test(S->I, source, metatableI_string2field(S, field), &mt);

    if (result) {
        stackI_push(S, mt);
    }

    return result;
}

MORPHINE_API void mapi_set_metatable(morphine_state_t S) {
    struct value source = stackI_peek(S, 1);
    struct value metatable = stackI_peek(S, 0);
    metatableI_set(S, source, valueI_safe_as_table(metatable, NULL));
    stackI_pop(S, 1);
}

MORPHINE_API void mapi_set_default_metatable(morphine_state_t S, const char *type) {
    struct value metatable = stackI_peek(S, 0);
    metatableI_set_default(S->I, valueI_string2type(S, type), valueI_safe_as_table(metatable, NULL));
}

MORPHINE_API void mapi_get_metatable(morphine_state_t S) {
    struct value *source = stackI_vector(S, 0, 1);
    *source = metatableI_get(S, *source);
}

MORPHINE_API void mapi_get_default_metatable(morphine_state_t S, const char *type) {
    stackI_push(S, metatableI_get_default(S->I, valueI_string2type(S, type)));
}

MORPHINE_API bool mapi_is_metatype(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);

    return valueI_is_table(value) || valueI_is_userdata(value);
}
