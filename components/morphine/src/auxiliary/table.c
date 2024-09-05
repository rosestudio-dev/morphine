//
// Created by why-iskra on 04.09.2024.
//

#include "morphine/auxiliary/table.h"
#include "morphine/api.h"

MORPHINE_AUX bool maux_table_get(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_table_get(U);
}

MORPHINE_AUX void maux_table_getoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_table_getoe(U);
}

MORPHINE_AUX void maux_table_set(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_rotate(U, 2);
    mapi_table_set(U);
}

MORPHINE_AUX bool maux_table_remove(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_table_remove(U);
}

MORPHINE_AUX void maux_table_removeoe(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    mapi_table_removeoe(U);
}
