//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/metatable.h"
#include "morphine/core/convert.h"
#include "morphine/core/instance.h"
#include "morphine/core/metatable/type.h"
#include "morphine/core/throw.h"
#include "morphine/gc/barrier.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include <string.h>

static inline struct value get_mask(morphine_instance_t I, struct table *metatable) {
    if (metatable == NULL) {
        return valueI_nil;
    }

    struct value field_name = valueI_object(I->metafields[MTYPE_METAFIELD_MASK]);

    bool has = false;
    struct value value = tableI_get(I, metatable, field_name, &has);
    if (has) {
        return value;
    }

    return valueI_object(metatable);
}

static inline bool get_lock(morphine_instance_t I, struct table *metatable) {
    if (metatable == NULL) {
        return false;
    }

    struct value field_name = valueI_object(I->metafields[MTYPE_METAFIELD_LOCK]);

    bool has = false;
    struct value value = tableI_get(I, metatable, field_name, &has);
    return has && valueI_tobool(value);
}

static inline bool metatable_test(
    morphine_instance_t I,
    struct value source,
    struct string *field,
    struct value *result
) {
    struct table *metatable = NULL;
    if (valueI_is_table(source)) {
        metatable = valueI_as_table(source)->metatable;
    } else if (valueI_is_userdata(source)) {
        metatable = valueI_as_userdata(source)->metatable;
    }

    struct value field_name = valueI_object(field);

    bool has = false;

    struct value extracted = valueI_nil;
    if (metatable != NULL) {
        extracted = tableI_get(I, metatable, field_name, &has);
    }

    if (result != NULL) {
        *result = extracted;
    }

    return has;
}

void metatableI_set(morphine_instance_t I, struct value value, struct table *metatable) {
    struct table **container;
    if (valueI_is_table(value)) {
        container = &valueI_as_table(value)->metatable;
    } else if (valueI_is_userdata(value)) {
        container = &valueI_as_userdata(value)->metatable;
    } else {
        throwI_errorf(I, "metatable cannot be set to %s", valueI_type(I, value, true));
    }

    if (get_lock(I, *container)) {
        throwI_error(I, "metatable was locked");
    }

    *container = gcI_objbarrier(I, valueI_as_object(value), metatable);
}

struct value metatableI_get(morphine_instance_t I, struct value value) {
    struct table *metatable = NULL;
    if (valueI_is_table(value)) {
        metatable = valueI_as_table(value)->metatable;
    } else if (valueI_is_userdata(value)) {
        metatable = valueI_as_userdata(value)->metatable;
    } else {
        throwI_errorf(I, "metatable cannot be get from %s", valueI_type(I, value, true));
    }

    return get_mask(I, metatable);
}

bool metatableI_test(morphine_instance_t I, struct value source, mtype_metafield_t field, struct value *result) {
    if (MORPHINE_METAFIELDS_START > field || field >= MORPHINE_METAFIELDS_COUNT) {
        throwI_panic(I, "unsupported meta field");
    }

    struct string *name = I->metafields[field];
    return metatable_test(I, source, name, result);
}

const char *metatableI_field2string(morphine_instance_t I, mtype_metafield_t field) {
    switch (field) {
#define mspec_metatable_field(n, s) case MTYPE_METAFIELD_##n: return MORPHINE_METAFIELD_PREFIX#s;

#include "morphine/core/metatable/specification.h"

#undef mspec_metatable_field
    }

    throwI_panic(I, "unsupported meta field");
}

mtype_metafield_t metatableI_string2field(morphine_instance_t I, const char *name) {
    for (mtype_metafield_t field = MORPHINE_METAFIELDS_START; field < MORPHINE_METAFIELDS_COUNT; field++) {
        if (strcmp(metatableI_field2string(I, field), name) == 0) {
            return field;
        }
    }

    throwI_errorf(I, "unknown type '%s'", name);
}
