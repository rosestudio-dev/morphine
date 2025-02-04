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
#include "morphine/misc/metatable/type.h"
#include <string.h>

static inline struct value extract_metatable(morphine_instance_t I, struct table *metatable) {
    if (metatable == NULL) {
        return valueI_nil;
    }

    struct value field_name = valueI_object(I->metatable.names[MORPHINE_METAFIELD_MASK]);

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

        if (table->lock.metatable) {
            throwI_error(I, "metatable was locked");
        }

        if (metatable != NULL) {
            gcI_objbarrier(I, table, metatable);
        }

        table->metatable = metatable;
        return;
    }

    if (valueI_is_userdata(value)) {
        struct userdata *userdata = valueI_as_userdata(value);

        if (userdata->lock.metatable) {
            throwI_error(I, "metatable was locked");
        }

        if (metatable != NULL) {
            gcI_objbarrier(I, userdata, metatable);
        }

        userdata->metatable = metatable;
        return;
    }

    throwI_errorf(I, "metatable cannot be set to %s", valueI_type(I, value, true));
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

    return extract_metatable(I, metatable);
}

bool metatableI_builtin_test(
    morphine_instance_t I,
    struct value source,
    morphine_metatable_field_t field,
    struct value *result
) {
    if (MORPHINE_METATABLE_FIELDS_START > field || field >= MORPHINE_METATABLE_FIELDS_COUNT) {
        throwI_panic(I, "unsupported meta field");
    }

    struct string *name = I->metatable.names[field];
    return metatableI_test(I, source, name, result);
}

bool metatableI_test(
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

const char *metatableI_field2string(morphine_instance_t I, morphine_metatable_field_t field) {
    switch (field) {
#define mspec_metatable_field(n, s) case MORPHINE_METAFIELD_##n: return MORPHINE_METATABLE_FIELD_PREFIX#s;

#include "morphine/misc/metatable/specification.h"

#undef mspec_metatable_field
    }

    throwI_panic(I, "unsupported meta field");
}

morphine_metatable_field_t metatableI_string2field(morphine_instance_t I, const char *name) {
    for (morphine_metatable_field_t field = MORPHINE_METATABLE_FIELDS_START;
         field < MORPHINE_METATABLE_FIELDS_COUNT; field++) {
        if (strcmp(metatableI_field2string(I, field), name) == 0) {
            return field;
        }
    }

    throwI_errorf(I, "unknown type '%s'", name);
}
