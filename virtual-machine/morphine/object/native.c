//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/native.h"
#include "morphine/core/alloc.h"
#include "morphine/core/throw.h"
#include "morphine/utils/unused.h"

struct native *nativeI_create(morphine_instance_t I, const char *name, morphine_native_t function) {
    size_t alloc_size = sizeof(struct native);

    struct native *result = allocI_uni(I, NULL, 0, alloc_size);

    (*result) = (struct native) {
        .function = function,
        .name = name,
        .registry_key = valueI_nil
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_NATIVE);

    return result;
}

void nativeI_free(morphine_instance_t I, struct native *native) {
    allocI_uni(
        I,
        native,
        sizeof(struct native),
        0
    );
}

size_t nativeI_allocated_size(struct native *native) {
    morphinem_unused(native);

    return sizeof(struct native);
}
