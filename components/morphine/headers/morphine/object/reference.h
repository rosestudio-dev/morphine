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
