//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/reference.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"

struct reference *referenceI_create(morphine_instance_t I, struct value value) {
    size_t rollback = gcI_safe_value(I, value);

    // create
    struct reference *result = allocI_uni(I, NULL, sizeof(struct reference));
    (*result) = (struct reference) {
        .value = value,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_REFERENCE);

    gcI_reset_safe(I, rollback);

    return result;
}

void referenceI_free(morphine_instance_t I, struct reference *reference) {
    allocI_free(I, reference);
}

struct value *referenceI_get(morphine_instance_t I, struct reference *reference) {
    if (reference == NULL) {
        throwI_error(I, "reference is null");
    }

    return &reference->value;
}

void referenceI_set(morphine_instance_t I, struct reference *reference, struct value value) {
    if (reference == NULL) {
        throwI_error(I, "reference is null");
    }

    reference->value = value;
}
