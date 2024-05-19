//
// Created by why-iskra on 19.05.2024.
//

#include <string.h>
#include "morphine/auxiliary/sio.h"
#include "morphine/api.h"

struct vector_data {
    bool hold;
    size_t size;
    size_t pointer;
    const uint8_t *vector;
};

static void *vector_open(morphine_sio_accessor_t A, void *data) {
    struct vector_data *vdata = data;

    size_t size = sizeof(struct vector_data);
    if (vdata->hold) {
        size += vdata->size;
    }

    struct vector_data *result = mapi_sio_accessor_alloc(A, NULL, size);
    *result = *vdata;

    if (vdata->hold) {
        uint8_t *pointer = ((uint8_t *) result) + sizeof(struct vector_data);
        result->vector = pointer;
        memcpy(pointer, vdata->vector, result->size);
    }

    return result;
}

static void vector_close(morphine_sio_accessor_t A, void *data) {
    struct vector_data *vdata = data;
    mapi_sio_accessor_free(A, vdata);
}

static size_t vector_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    (void) A;

    struct vector_data *vdata = data;

    size_t read = 0;
    for (size_t i = 0; i < size; i++) {
        if (vdata->pointer >= vdata->size) {
            buffer[i] = 0;
        } else {
            buffer[i] = vdata->vector[vdata->pointer];
            vdata->pointer++;
            read++;
        }
    }

    return read;
}

MORPHINE_AUX void maux_push_sio_vector(
    morphine_coroutine_t U, const uint8_t *vector, size_t size, bool hold
) {
    struct vector_data data = {
        .hold = hold,
        .size = size,
        .pointer = 0,
        .vector = vector
    };

    morphine_sio_interface_t interface = {
        .open = vector_open,
        .close = vector_close,
        .read = vector_read,
        .write = NULL,
        .flush = NULL,
    };

    mapi_push_sio(U, interface);
    mapi_sio_open(U, &data);
}
