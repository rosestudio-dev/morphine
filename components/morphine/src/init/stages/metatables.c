//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"

static void init_metatable_names(morphine_instance_t I) {
    for (enum metatable_field field = MFS_START; field < MFS_COUNT; field++) {
        const char *name = metatableI_field2string(I, field);
        I->metatable.names[field] = stringI_create(I, name);
    }
}

static void init_metatable_defaults(morphine_instance_t I) {
    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        I->metatable.defaults[type] = NULL;
    }
}

void init_metatables(morphine_instance_t I) {
    init_metatable_defaults(I);
    init_metatable_names(I);
}
