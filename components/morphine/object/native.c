//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

struct native *nativeI_create(morphine_instance_t I, const char *name, morphine_native_t function) {
    if (name == NULL) {
        throwI_error(I, "Native name is null");
    }

    if (function == NULL) {
        throwI_error(I, "Native function is null");
    }

    size_t name_len = strlen(name);

    if (name_len > MLIMIT_NATIVE_NAME) {
        throwI_error(I, "Native name too big");
    }

    size_t alloc_size = sizeof(struct native) + ((name_len + 1) * sizeof(char));
    struct native *result = allocI_uni(I, NULL, alloc_size);

    (*result) = (struct native) {
        .function = function,
        .name = ((void *) result) + sizeof(struct native),
        .name_len = name_len,
        .registry_key = valueI_nil
    };

    memset(result->name, 0, (name_len + 1) * sizeof(char));
    strcpy(result->name, name);

    objectI_init(I, objectI_cast(result), OBJ_TYPE_NATIVE);

    return result;
}

void nativeI_free(morphine_instance_t I, struct native *native) {
    allocI_free(I, native);
}
