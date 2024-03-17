//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct reference {
    struct object header;

    struct value value;
};

struct reference *referenceI_create(morphine_instance_t, struct value);
void referenceI_free(morphine_instance_t, struct reference *);

struct value *referenceI_get(morphine_instance_t, struct reference *);
void referenceI_set(morphine_instance_t, struct reference *, struct value value);

size_t referenceI_allocated_size(struct reference *);

static inline void referenceI_invalidate(struct reference *reference) {
    struct object *object = valueI_safe_as_object(reference->value, NULL);

    if (object == NULL) {
        return;
    }

    if (!object->mark) {
        reference->value = valueI_nil;
    }
}
