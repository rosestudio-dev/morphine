//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/native.h"
#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"
#include "morphine/utils/unused.h"

struct native *nativeI_create(morphine_instance_t I, const char *name, morphine_native_t function) {
    if (name == NULL) {
        throwI_message_panic(I, NULL, "Native name is null");
    }

    if (function == NULL) {
        throwI_message_panic(I, NULL, "Native function is null");
    }

    struct native *result = allocI_uni(I, NULL, sizeof(struct native));

    (*result) = (struct native) {
        .function = function,
        .name = name,
        .registry_key = valueI_nil
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_NATIVE);

    return result;
}

void nativeI_free(morphine_instance_t I, struct native *native) {
    allocI_free(I, native);
}

size_t nativeI_allocated_size(void) {
    return sizeof(struct native);
}
