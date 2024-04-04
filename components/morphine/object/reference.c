//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/reference.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

struct reference *referenceI_create(morphine_instance_t I, struct value value) {
    size_t alloc_size = sizeof(struct reference);

    struct reference *result = allocI_uni(I, NULL, alloc_size);

    (*result) = (struct reference) {
        .value = value,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_REFERENCE);

    return result;
}

void referenceI_free(morphine_instance_t I, struct reference *reference) {
    allocI_free(I, reference);
}

struct value *referenceI_get(morphine_instance_t I, struct reference *reference) {
    if (reference == NULL) {
        throwI_error(I, "Reference is null");
    }

    return &reference->value;
}

void referenceI_set(morphine_instance_t I, struct reference *reference, struct value value) {
    if (reference == NULL) {
        throwI_error(I, "Reference is null");
    }

    reference->value = value;
}
