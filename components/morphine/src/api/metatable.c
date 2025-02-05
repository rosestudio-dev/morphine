//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/misc/metatable.h"
#include "morphine/object/string.h"
#include "morphine/gc/safe.h"

MORPHINE_API bool mapi_metatable_test(morphine_coroutine_t U, const char *field) {
    struct value source = stackI_peek(U, 0);

    gcI_safe_enter(U->I);
    
    struct string *string = gcI_safe_obj(U->I, string, stringI_create(U->I, field));

    struct value mt;
    bool result = metatableI_test(U->I, source, string, &mt);

    if (result) {
        stackI_push(U, mt);
    }

    gcI_safe_exit(U->I);

    return result;
}

MORPHINE_API bool mapi_metatable_builtin_test(morphine_coroutine_t U, mtype_metafield_t field) {
    struct value source = stackI_peek(U, 0);

    struct value mt;
    bool result = metatableI_builtin_test(U->I, source, field, &mt);

    if (result) {
        stackI_push(U, mt);
    }

    return result;
}

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
