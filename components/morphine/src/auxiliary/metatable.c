//
// Created by why-iskra on 02.11.2024.
//

#include "morphine/auxiliary/metatable.h"

MORPHINE_AUX const char *maux_metafield_name(morphine_metatable_field_t field) {
    switch (field) {
#define mspec_metatable_field(n, s) case MORPHINE_METAFIELD_##n: return MORPHINE_METATABLE_FIELD_PREFIX#s;

#include "morphine/misc/metatable/specification.h"

#undef mspec_metatable_field
    }

    return NULL;
}
