//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/misc/metatable.h"

MORPHINE_API bool mapi_metatable_test(morphine_coroutine_t U, const char *field) {
    struct value source = stackI_peek(U, 0);

    struct value mt;
    bool result = metatableI_test(U->I, source, metatableI_string2field(U->I, field), &mt);

    if (result) {
        stackI_push(U, mt);
    }

    return result;
}

MORPHINE_API void mapi_set_metatable(morphine_coroutine_t U) {
    struct value source = stackI_peek(U, 1);
    struct value metatable = stackI_peek(U, 0);
    metatableI_set(U->I, source, valueI_safe_as_table(metatable, NULL));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_set_default_metatable(morphine_coroutine_t U, const char *type) {
    struct value metatable = stackI_peek(U, 0);
    metatableI_set_default(U->I, valueI_string2type(U->I, type), valueI_safe_as_table(metatable, NULL));
}

MORPHINE_API void mapi_get_metatable(morphine_coroutine_t U) {
    struct value source = stackI_peek(U, 0);
    struct value result = metatableI_get(U->I, source);
    stackI_push(U, result);
}

MORPHINE_API void mapi_get_default_metatable(morphine_coroutine_t U, const char *type) {
    stackI_push(U, metatableI_get_default(U->I, valueI_string2type(U->I, type)));
}

MORPHINE_API bool mapi_is_metatype(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);

    return valueI_is_table(value) || valueI_is_userdata(value);
}
