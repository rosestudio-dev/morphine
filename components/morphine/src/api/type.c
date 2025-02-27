//
// Created by why-iskra on 22.07.2024.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/value.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/usertype.h"

MORPHINE_API void mapi_type_declare(
    morphine_instance_t I,
    const char *name,
    size_t allocate,
    bool require_metatable,
    morphine_userdata_constructor_t constructor,
    morphine_userdata_destructor_t destructor,
    morphine_userdata_compare_t compare,
    morphine_userdata_hash_t hash
) {
    usertypeI_declare(
        I, name, allocate,
        require_metatable,
        constructor,
        destructor,
        compare,
        hash
    );
}

MORPHINE_API bool mapi_type_is_declared(morphine_instance_t I, const char *name) {
    return usertypeI_is_declared(I, name);
}

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

    if (strcmp(type, "size") == 0) {
        return mapi_is_size(U);
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

MORPHINE_API bool mapi_is_size(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_size(value);
}
