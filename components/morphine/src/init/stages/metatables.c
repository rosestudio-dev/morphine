//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"

static void init_metatable_names(morphine_instance_t I) {
    for (morphine_metatable_field_t field = MORPHINE_METATABLE_FIELDS_START;
         field < MORPHINE_METATABLE_FIELDS_COUNT; field++) {
        const char *name = metatableI_field2string(I, field);
        I->metatable.names[field] = stringI_create(I, name);
    }
}

void init_metatables(morphine_instance_t I) {
    init_metatable_names(I);
}
