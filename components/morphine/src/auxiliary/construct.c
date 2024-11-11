//
// Created by why-iskra on 02.11.2024.
//

#include <string.h>
#include "morphine/auxiliary/construct.h"
#include "morphine/api.h"

static void access(morphine_coroutine_t U, const char *name) {
    mapi_peek(U, 0);

    size_t from = 0;
    size_t len = strlen(name);
    for (size_t i = 0; i < len; i++) {
        if (name[i] == '.') {
            mapi_push_stringn(U, name + from, i - from);
            if (mapi_table_get(U)) {
                if (!mapi_is_type(U, "table")) {
                    mapi_error(U, "intersection of names");
                }

                mapi_rotate(U, 2);
            } else {
                mapi_push_table(U);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mapi_peek(U, 0);
                mapi_rotate(U, 3);

                mapi_push_stringn(U, name + from, i - from);
                mapi_rotate(U, 2);
                mapi_table_set(U);
            }
            mapi_pop(U, 1);

            from = i + 1;
        }
    }

    mapi_push_stringn(U, name + from, len - from);
}

MORPHINE_AUX void maux_construct(morphine_coroutine_t U, maux_construct_element_t *element) {
    mapi_push_table(U);

    for (; element->type != MAUX_CONSTRUCT_TYPE_END; element++) {
        access(U, element->name);

        switch (element->type) {
            case MAUX_CONSTRUCT_TYPE_STRING:
                mapi_push_string(U, element->value.string);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_INTEGER:
                mapi_push_integer(U, element->value.integer);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_DECIMAL:
                mapi_push_decimal(U, element->value.decimal);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_BOOLEAN:
                mapi_push_boolean(U, element->value.boolean);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_FUNCTION:
                mapi_push_string(U, element->value.function.name);
                mapi_push_native(U, element->value.function.value);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_SIZE:
                mapi_push_size(U, element->value.size.value, element->value.size.name);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_NIL:
                mapi_push_nil(U);
                mapi_table_set(U);
                break;
            case MAUX_CONSTRUCT_TYPE_END:
            case MAUX_CONSTRUCT_TYPE_EMPTY:
                mapi_pop(U, 1);
        }

        mapi_pop(U, 1);
    }
}
