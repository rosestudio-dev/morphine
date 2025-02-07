//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/reference.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"

struct reference *referenceI_create(morphine_instance_t I, struct value value) {
    gcI_safe_enter(I);
    gcI_safe(I, value);

    // create
    struct reference *result = allocI_uni(I, NULL, sizeof(struct reference));
    (*result) = (struct reference) {
        .value = value,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_REFERENCE);

    gcI_safe_exit(I);

    return result;
}

void referenceI_free(morphine_instance_t I, struct reference *reference) {
    allocI_free(I, reference);
}

struct value *referenceI_get(struct reference *reference) {
    return &reference->value;
}

void referenceI_set(struct reference *reference, struct value value) {
    reference->value = value;
}
