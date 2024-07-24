//
// Created by why-iskra on 22.07.2024.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/value.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/usertype.h"

MORPHINE_API void mapi_type_declare(
    morphine_instance_t I,
    const char *name,
    size_t allocate,
    morphine_free_t free,
    bool require_metatable
) {
    if (!usertypeI_declare(I, name, allocate, free, require_metatable)) {
        throwI_error(I, "type is already declared");
    }
}

MORPHINE_API bool mapi_type_try_declare(
    morphine_instance_t I,
    const char *name,
    size_t allocate,
    morphine_free_t free,
    bool require_metatable
) {
    return usertypeI_declare(I, name, allocate, free, require_metatable);
}

MORPHINE_API bool mapi_type_undeclare(morphine_instance_t I, const char *name) {
    return usertypeI_undeclare(I, name);
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

    if (strcmp(type, "index") == 0) {
        return mapi_is_index(U);
    }

    return mapi_is_type(U, type);
}

MORPHINE_API bool mapi_is_type(morphine_coroutine_t U, const char *type) {
    return strcmp(valueI_type(U->I, stackI_peek(U, 0), false), type) == 0;
}

MORPHINE_API bool mapi_is_callable(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return callstackI_is_callable(U->I, value);
}

MORPHINE_API bool mapi_is_metatype(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_table(value) || valueI_is_userdata(value);
}

MORPHINE_API bool mapi_is_iterable(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_table(value) || valueI_is_vector(value);
}

MORPHINE_API bool mapi_is_size(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_size(value);
}

MORPHINE_API bool mapi_is_index(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return valueI_is_index(value);
}
