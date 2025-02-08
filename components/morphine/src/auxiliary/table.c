//
// Created by why-iskra on 04.09.2024.
//

#include <string.h>
#include "morphine/auxiliary/table.h"
#include "morphine/api.h"

MORPHINE_AUX bool maux_table_access(morphine_coroutine_t U, const char *key) {
    size_t from = 0;
    size_t len = strlen(key);
    for (size_t i = 0; i < len; i++) {
        if (key[i] == '.') {
            mapi_push_stringn(U, key + from, i - from);
            if (mapi_table_get(U)) {
                if (!mapi_is_type(U, "table")) {
                    mapi_error(U, "resulting subkey value isn't table");
                }

                mapi_rotate(U, 2);
                mapi_pop(U, 1);
            } else {
                mapi_error(U, "access by subkey isn't possible");
            }

            from = i + 1;
        }
    }

    mapi_push_stringn(U, key + from, len - from);
    bool has = mapi_table_get(U);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    return has;
}

MORPHINE_AUX bool maux_table_has(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    bool result = mapi_table_get(U);
    mapi_pop(U, 1);
    return result;
}

MORPHINE_AUX bool maux_table_get(morphine_coroutine_t U, const char *key) {
    mapi_push_string(U, key);
    return mapi_table_get(U);
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
