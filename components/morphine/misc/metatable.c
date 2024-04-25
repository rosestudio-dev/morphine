//
// Created by whyiskra on 16.12.23.
//

#include "morphine/misc/metatable.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/coroutine.h"
#include "morphine/gc/barrier.h"
#include <string.h>

static inline struct value extract_metatable(morphine_instance_t I, struct table *metatable) {
    if (metatable == NULL) {
        return valueI_nil;
    }

    struct value field_name = valueI_object(I->metatable.names[MF_MASK]);

    bool has = false;
    struct value mask_value = tableI_get(I, metatable, field_name, &has);

    if (has) {
        return mask_value;
    } else {
        return valueI_object(metatable);
    }
}

void metatableI_set(morphine_instance_t I, struct value value, struct table *metatable) {
    if (valueI_is_table(value)) {
        struct table *table = valueI_as_table(value);
        if (metatable != NULL) {
            gcI_objbarrier(I, table, metatable);
        }
        table->metatable = metatable;
        return;
    }

    if (valueI_is_userdata(value)) {
        struct userdata *userdata = valueI_as_userdata(value);
        if (metatable != NULL) {
            gcI_objbarrier(I, userdata, metatable);
        }
        userdata->metatable = metatable;
        return;
    }

    throwI_errorf(I, "Metatable cannot be set to %s", valueI_type2string(I, value.type));
}

void metatableI_set_default(morphine_instance_t I, enum value_type type, struct table *metatable) {
    if (VALUE_TYPES_START > type || type >= VALUE_TYPES_COUNT) {
        throwI_panic(I, "Unsupported value type");
    }

    I->metatable.defaults[type] = metatable;
}

struct value metatableI_get_default(morphine_instance_t I, enum value_type type) {
    if (VALUE_TYPES_START > type || type >= VALUE_TYPES_COUNT) {
        throwI_panic(I, "Unsupported value type");
    }

    return extract_metatable(I, I->metatable.defaults[type]);
}

struct value metatableI_get(morphine_instance_t I, struct value value) {
    struct table *metatable = NULL;
    if (valueI_is_table(value)) {
        metatable = valueI_as_table(value)->metatable;
    } else if (valueI_is_userdata(value)) {
        metatable = valueI_as_userdata(value)->metatable;
    } else {
        throwI_errorf(I, "Metatable cannot be get from %s", valueI_type2string(I, value.type));
    }

    return extract_metatable(I, metatable);
}

bool metatableI_test(
    morphine_instance_t I,
    struct value source,
    enum metatable_field field,
    struct value *result
) {
    if (MFS_START > field || field >= MFS_COUNT) {
        throwI_panic(I, "Unsupported meta field");
    }

    struct table *metatable;
    if (valueI_is_table(source)) {
        metatable = valueI_as_table(source)->metatable;
    } else if (valueI_is_userdata(source)) {
        metatable = valueI_as_userdata(source)->metatable;
    } else {
        metatable = I->metatable.defaults[source.type];
    }

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
        case MF_ITERATOR:
            return "_mf_iterator";
        case MF_ITERATOR_INIT:
            return "_mf_iterator_init";
        case MF_ITERATOR_HAS:
            return "_mf_iterator_has";
        case MF_ITERATOR_NEXT:
            return "_mf_iterator_next";
    }

    throwI_panic(I, "Unsupported meta field");
}

enum metatable_field metatableI_string2field(morphine_instance_t I, const char *name) {
    for (enum metatable_field field = MFS_START; field < MFS_COUNT; field++) {
        if (strcmp(metatableI_field2string(I, field), name) == 0) {
            return field;
        }
    }

    throwI_errorf(I, "Unknown type '%s'", name);
}
