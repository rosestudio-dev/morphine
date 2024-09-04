//
// Created by why-iskra on 04.09.2024.
//

#include "morphine/auxiliary/registry.h"
#include "morphine/api.h"

MORPHINE_AUX bool maux_registry_get(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_registry_get(U);
}

MORPHINE_AUX void maux_registry_getoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_registry_getoe(U);
}

MORPHINE_AUX void maux_registry_set(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_rotate(U, 2);
    mapi_registry_set(U);
}

MORPHINE_AUX bool maux_registry_remove(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_registry_remove(U);
}

MORPHINE_AUX void maux_registry_removeoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_registry_removeoe(U);
}
