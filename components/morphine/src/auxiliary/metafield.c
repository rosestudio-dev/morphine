//
// Created by why-iskra on 02.11.2024.
//

#include <string.h>
#include "morphine/auxiliary/metafield.h"
#include "morphine/api.h"

MORPHINE_AUX const char *maux_metafield_name(morphine_coroutine_t U, morphine_metatable_field_t field) {
    switch (field) {
#define mspec_metatable_field(n, s) case MORPHINE_METAFIELD_##n: return MORPHINE_METATABLE_FIELD_PREFIX#s;

#include "morphine/misc/metatable/specification.h"

#undef mspec_metatable_field
    }

    mapi_error(U, "undefined metatable field");
}

MORPHINE_AUX morphine_metatable_field_t maux_metafield_from_name(morphine_coroutine_t U, const char *name) {
    for (morphine_metatable_field_t field = MORPHINE_METATABLE_FIELDS_START; field < MORPHINE_METATABLE_FIELDS_COUNT; field++) {
        if (strcmp(maux_metafield_name(U, field), name) == 0) {
            return field;
        }
    }

    mapi_error(U, "undefined metatable field");
}
