//
// Created by whyiskra on 07.01.24.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc/localstorage.h"

MORPHINE_API bool mapi_localstorage_get(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = localstorageI_get(U, key, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API void mapi_localstorage_getoe(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = localstorageI_get(U, key, &has);

    if (has) {
        stackI_replace(U, 0, result);
    } else {
        throwI_error(U->I, "cannot get value from localstorage by key");
    }
}

MORPHINE_API void mapi_localstorage_set(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    localstorageI_set(U, key, value);

    stackI_pop(U, 2);
}

MORPHINE_API bool mapi_localstorage_remove(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = localstorageI_remove(U, key, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API void mapi_localstorage_removeoe(morphine_coroutine_t U) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = localstorageI_remove(U, key, &has);

    if (has) {
        stackI_replace(U, 0, result);
    } else {
        throwI_error(U->I, "cannot remove value from localstorage by key");
    }
}

MORPHINE_API void mapi_localstorage_clear(morphine_coroutine_t U) {
    localstorageI_clear(U);
}
