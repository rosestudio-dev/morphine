//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/binary/loader.h"
#include "morphine/utils/unused.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include "morphine/object/coroutine.h"
#include <stddef.h>

struct loader_array {
    size_t size;
    size_t pointer;
    const uint8_t *vector;
};

static void *loader_array_open(morphine_coroutine_t U, void *data) {
    struct loader_array *array = cast(struct loader_array *, data);

    if (array->size == 0 || array->vector == NULL) {
        throwI_error(U->I, "Binary vector is empty");
    }

    array->pointer = 0;

    return array;
}

static uint8_t loader_array_read(morphine_coroutine_t U, void *data, const char **error) {
    unused(U);

    struct loader_array *loader = cast(struct loader_array *, data);

    if (loader->pointer >= loader->size) {
        *error = "Binary corrupted";
        return 0;
    }

    return loader->vector[loader->pointer++];
}

MORPHINE_API void mapi_rload(morphine_coroutine_t U, size_t size, const uint8_t *vector) {
    struct loader_array array = {
        .vector = vector,
        .size = size
    };

    struct function *result = loaderI_load(U, loader_array_open, loader_array_read, NULL, cast(void *, &array));
    stackI_push(U, valueI_object(result));
}

MORPHINE_API void mapi_load(
    morphine_coroutine_t U,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
) {
    struct function *result = loaderI_load(U, init, read, finish, args);
    stackI_push(U, valueI_object(result));
}
