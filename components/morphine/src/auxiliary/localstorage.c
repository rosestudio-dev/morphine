//
// Created by why-iskra on 04.09.2024.
//

#include "morphine/auxiliary/localstorage.h"
#include "morphine/api.h"

MORPHINE_AUX bool maux_localstorage_get(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_localstorage_get(U);
}

MORPHINE_AUX void maux_localstorage_getoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_localstorage_getoe(U);
}

MORPHINE_AUX void maux_localstorage_set(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_rotate(U, 2);
    mapi_localstorage_set(U);
}

MORPHINE_AUX bool maux_localstorage_remove(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_localstorage_remove(U);
}

MORPHINE_AUX void maux_localstorage_removeoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_localstorage_removeoe(U);
}
