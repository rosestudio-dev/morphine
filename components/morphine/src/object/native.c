//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"

struct native *nativeI_create(morphine_instance_t I, struct string *name, morphine_native_t function) {
    if (name == NULL) {
        throwI_error(I, "native name is null");
    }

    if (function == NULL) {
        throwI_error(I, "native function is null");
    }

    size_t rollback = gcI_safe_obj(I, objectI_cast(name));

    // create
    struct native *result = allocI_uni(I, NULL, sizeof(struct native));
    (*result) = (struct native) {
        .function = function,
        .name = name
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_NATIVE);

    gcI_reset_safe(I, rollback);

    return result;
}

void nativeI_free(morphine_instance_t I, struct native *native) {
    allocI_free(I, native);
}
