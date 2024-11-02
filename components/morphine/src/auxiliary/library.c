//
// Created by why-iskra on 02.11.2024.
//

#include <string.h>
#include "morphine/auxiliary/library.h"
#include "morphine/auxiliary/table.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_library_access(morphine_coroutine_t U, const char *key) {
    size_t size = 0;
    size_t len = strlen(key);
    for (; size < len; size++) {
        if (key[size] == '.') {
            break;
        }
    }

    mapi_push_stringn(U, key, size);
    mapi_library(U, mapi_get_cstr(U));

    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    if (size != len) {
        if (!maux_table_access(U, key + size + 1)) {
            mapi_error(U, "subkey not found in library");
        }
    }
}
