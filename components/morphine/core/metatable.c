//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/metatable.h"
#include "morphine/core/object.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/state.h"
#include "morphine/gc/barrier.h"
#include <string.h>

void metatableI_set(morphine_state_t S, struct value value, struct table *metatable) {
    {
        struct table *table = valueI_safe_as_table(value, NULL);
        if (table != NULL) {
            if (metatable != NULL) {
                gcI_objbarrier(table, metatable);
            }
            table->metatable = metatable;
            return;
        }
    }

    {
        struct userdata *userdata = valueI_safe_as_userdata(value, NULL);
        if (userdata != NULL) {
            if (metatable != NULL) {
                gcI_objbarrier(userdata, metatable);
            }
            userdata->metatable = metatable;
            return;
        }
    }

    throwI_errorf(S, "Metatable can be set to %s", valueI_type2string(S->I, value.type));
}

void metatableI_set_default(morphine_instance_t I, enum value_type type, struct table *metatable) {
    I->metatable.defaults[type] = metatable;
}

struct value metatableI_get_default(morphine_instance_t I, enum value_type type) {
    struct table *table = I->metatable.defaults[type];

    if (table == NULL) {
        return valueI_nil;
    } else {
        return valueI_object(table);
    }
}

static inline struct table *get_metatable(morphine_instance_t I, struct value value) {
    struct table *metatable = NULL;

    if (valueI_is_table(value)) {
        metatable = valueI_as_table(value)->metatable;
    } else if (valueI_is_userdata(value)) {
        metatable = valueI_as_userdata(value)->metatable;
    } else {
        metatable = I->metatable.defaults[value.type];
    }

    return metatable;
}

struct value metatableI_get(morphine_state_t S, struct value value) {
    struct table *metatable = get_metatable(S->I, value);

    if (metatable == NULL) {
        return valueI_nil;
    }

    struct value field_name = valueI_object(S->I->metatable.names[MF_MASK]);

    bool has = false;
    struct value lock_value = tableI_get(S->I, metatable, field_name, &has);

    if (has) {
        return lock_value;
    } else {
        return valueI_object(metatable);
    }
}

bool metatableI_test(morphine_instance_t I, struct value source, enum metatable_field field, struct value *result) {
    struct table *metatable = get_metatable(I, source);
    struct value field_name = valueI_object(I->metatable.names[field]);

    bool has = false;
    if (result != NULL) {
        if (metatable == NULL) {
            *result = valueI_nil;
        } else {
            *result = tableI_get(I, metatable, field_name, &has);
        }
    } else if (metatable != NULL) {
        tableI_get(I, metatable, field_name, &has);
    }

    return has;
}

const char *metatableI_field2string(morphine_instance_t I, enum metatable_field field) {
    switch (field) {
        case MF_CALL:
            return "_mf_call";
        case MF_GET:
            return "_mf_get";
        case MF_SET:
            return "_mf_set";
        case MF_TO_STRING:
            return "_mf_to_string";
        case MF_EQUAL:
            return "_mf_equal";
        case MF_MASK:
            return "_mf_mask";
        case MF_TYPE:
            return "_mf_type";
        case MF_ADD:
            return "_mf_add";
        case MF_SUB:
            return "_mf_sub";
        case MF_MUL:
            return "_mf_mul";
        case MF_DIV:
            return "_mf_div";
        case MF_MOD:
            return "_mf_mod";
        case MF_LESS:
            return "_mf_less";
        case MF_LESS_EQUAL:
            return "_mf_less_equal";
        case MF_OR:
            return "_mf_or";
        case MF_AND:
            return "_mf_and";
        case MF_CONCAT:
            return "_mf_concat";
        case MF_NEGATE:
            return "_mf_negate";
        case MF_NOT:
            return "_mf_not";
        case MF_LENGTH:
            return "_mf_length";
        case MF_REF:
            return "_mf_ref";
        case MF_DEREF:
            return "_mf_deref";
        case MF_GC:
            return "_mf_gc";
    }

    throwI_message_panic(I, NULL, "Unsupported meta field");
}

enum metatable_field metatableI_string2field(morphine_state_t S, const char *name) {
    for (enum metatable_field field = MFS_START; field < MFS_COUNT; field++) {
        if (strcmp(metatableI_field2string(S->I, field), name) == 0) {
            return field;
        }
    }

    throwI_errorf(S, "Unknown type '%s'", name);
}
