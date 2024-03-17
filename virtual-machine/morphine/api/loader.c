//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/loader.h"
#include "morphine/utils/unused.h"
#include "morphine/core/throw.h"
#include <stddef.h>

struct loader_array {
    size_t size;
    size_t pointer;
    const uint8_t *vector;
};

static void *loader_array_open(morphine_state_t S, void *data) {
    struct loader_array *array = morphinem_cast(struct loader_array *, data);

    if (array->size == 0 || array->vector == NULL) {
        throwI_message_error(S, "Binary vector is empty");
    }

    array->pointer = 0;

    return array;
}

static uint8_t loader_array_read(morphine_state_t S, void *data, const char **error) {
    morphinem_unused(S);

    struct loader_array *loader = morphinem_cast(struct loader_array *, data);

    if (loader->pointer >= loader->size) {
        *error = "Binary corrupted";
        return 0;
    }

    return loader->vector[loader->pointer++];
}

MORPHINE_API void mapi_rload(morphine_state_t S, size_t size, const uint8_t *vector) {
    struct loader_array array = {
        .vector = vector,
        .size = size
    };

    struct proto *result = loaderI_load(S, loader_array_open, loader_array_read, NULL, morphinem_cast(void *, &array));
    stackI_push(S, valueI_object(result));
}

MORPHINE_API void mapi_load(
    morphine_state_t S,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
) {
    struct proto *result = loaderI_load(S, init, read, finish, args);
    stackI_push(S, valueI_object(result));
}
