//
// Created by whyiskra on 07.01.24.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc/sharedstorage.h"

MORPHINE_API bool mapi_sharedstorage_get(morphine_coroutine_t U, const char *sharedkey) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = sharedstorageI_get(U->I, sharedkey, key, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API void mapi_sharedstorage_getoe(morphine_coroutine_t U, const char *sharedkey) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = sharedstorageI_get(U->I, sharedkey, key, &has);

    if (has) {
        stackI_replace(U, 0, result);
    } else {
        throwI_error(U->I, "cannot get value from sharedstorage by key");
    }
}

MORPHINE_API void mapi_sharedstorage_set(morphine_coroutine_t U, const char *sharedkey) {
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    sharedstorageI_set(U->I, sharedkey, key, value);

    stackI_pop(U, 2);
}

MORPHINE_API bool mapi_sharedstorage_remove(morphine_coroutine_t U, const char *sharedkey) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = sharedstorageI_remove(U->I, sharedkey, key, &has);
    stackI_replace(U, 0, result);

    return has;
}

MORPHINE_API void mapi_sharedstorage_removeoe(morphine_coroutine_t U, const char *sharedkey) {
    struct value key = stackI_peek(U, 0);

    bool has = false;
    struct value result = sharedstorageI_remove(U->I, sharedkey, key, &has);

    if (has) {
        stackI_replace(U, 0, result);
    } else {
        throwI_error(U->I, "cannot remove value from sharedstorage by key");
    }
}

MORPHINE_API void mapi_sharedstorage_clear(morphine_coroutine_t U, const char *sharedkey) {
    sharedstorageI_clear(U->I, sharedkey);
}
