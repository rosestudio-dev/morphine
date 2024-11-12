//
// Created by why-iskra on 04.09.2024.
//

#include "morphine/auxiliary/sharedstorage.h"
#include "morphine/api.h"

MORPHINE_AUX bool maux_sharedstorage_get(morphine_coroutine_t U, const char *sharedkey, const char *key) {
    mapi_push_string(U, key);
    return mapi_sharedstorage_get(U, sharedkey);
}

MORPHINE_AUX void maux_sharedstorage_getoe(morphine_coroutine_t U, const char *sharedkey, const char *key) {
    mapi_push_string(U, key);
    mapi_sharedstorage_getoe(U, sharedkey);
}

MORPHINE_AUX void maux_sharedstorage_set(morphine_coroutine_t U, const char *sharedkey, const char *key) {
    mapi_push_string(U, key);
    mapi_rotate(U, 2);
    mapi_sharedstorage_set(U, sharedkey);
}

MORPHINE_AUX bool maux_sharedstorage_remove(morphine_coroutine_t U, const char *sharedkey, const char *key) {
    mapi_push_string(U, key);
    return mapi_sharedstorage_remove(U, sharedkey);
}

MORPHINE_AUX void maux_sharedstorage_removeoe(morphine_coroutine_t U, const char *sharedkey, const char *key) {
    mapi_push_string(U, key);
    mapi_sharedstorage_removeoe(U, sharedkey);
}
