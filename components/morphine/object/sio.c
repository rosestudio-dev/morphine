//
// Created by why-iskra on 18.05.2024.
//

#include <stdio.h>
#include <string.h>
#include "morphine/object/sio.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/object/string.h"
#include "morphine/gc/safe.h"

struct sio_accessor {
    morphine_instance_t I;
};

struct sio *sioI_create(morphine_instance_t I, morphine_sio_interface_t interface) {
    if (interface.write == NULL && interface.read == NULL) {
        throwI_error(I, "SIO interface hasn't read/write functions");
    }

    struct sio *result = allocI_uni(I, NULL, sizeof(struct sio));

    (*result) = (struct sio) {
        .interface = interface,
        .opened = false,
        .data = NULL
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_SIO);

    return result;
}

static struct sio_accessor get_accessor(morphine_instance_t I) {
    return (struct sio_accessor) {
        .I = I
    };
}

static void close(morphine_instance_t I, struct sio *sio, bool require) {
    if (require && !sio->opened) {
        throwI_error(I, "SIO already closed");
    }

    if (sio->opened && sio->interface.close != NULL) {
        struct sio_accessor A = get_accessor(I);
        sio->interface.close(&A, sio->data);
    }
}

void sioI_free(morphine_instance_t I, struct sio *sio) {
    close(I, sio, false);
    allocI_free(I, sio);
}

void sioI_open(morphine_instance_t I, struct sio *sio, void *arg) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    if (sio->opened) {
        throwI_error(I, "SIO is already opened");
    }

    if (sio->interface.open == NULL) {
        sio->data = arg;
    } else {
        struct sio_accessor A = get_accessor(I);
        sio->data = sio->interface.open(&A, arg);
    }

    sio->opened = true;
}

bool sioI_is_opened(morphine_instance_t I, struct sio *sio) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    return sio->opened;
}

void sioI_close(morphine_instance_t I, struct sio *sio) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    close(I, sio, true);
}

size_t sioI_read(morphine_instance_t I, struct sio *sio, uint8_t *buffer, size_t size) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    if (sio->interface.read == NULL) {
        throwI_error(I, "SIO is write only");
    }

    if (!sio->opened) {
        throwI_error(I, "SIO isn't opened");
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.read(&A, sio->data, buffer, size);
}

size_t sioI_write(morphine_instance_t I, struct sio *sio, const uint8_t *buffer, size_t size) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    if (sio->interface.write == NULL) {
        throwI_error(I, "SIO is read only");
    }

    if (!sio->opened) {
        throwI_error(I, "SIO isn't opened");
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.write(&A, sio->data, buffer, size);
}

void sioI_flush(morphine_instance_t I, struct sio *sio) {
    if (sio == NULL) {
        throwI_error(I, "SIO is null");
    }

    if (!sio->opened) {
        throwI_error(I, "SIO isn't opened");
    }

    if (sio->interface.flush != NULL) {
        struct sio_accessor A = get_accessor(I);
        return sio->interface.flush(&A, sio->data);
    }
}

size_t sioI_print(morphine_instance_t I, struct sio *sio, const char *str) {
    return sioI_write(I, sio, (const uint8_t *) str, strlen(str));
}

size_t sioI_printf(morphine_instance_t I, struct sio *sio, const char *str, ...) {
    va_list args;
    va_start(args, str);
    size_t written = sioI_vprintf(I, sio, str, args);
    va_end(args);

    return written;
}

size_t sioI_vprintf(morphine_instance_t I, struct sio *sio, const char *str, va_list args) {
    struct string *result = stringI_createva(I, str, args);
    size_t rollback = gcI_safe_obj(I, objectI_cast(result));
    size_t written = sioI_write(I, sio, (const uint8_t *) result->chars, result->size);
    gcI_reset_safe(I, rollback);

    return written;
}

void *sioI_accessor_alloc(morphine_sio_accessor_t A, void *pointer, size_t size) {
    return allocI_uni(A->I, pointer, size);
}

void *sioI_accessor_alloc_vec(morphine_sio_accessor_t A, void *pointer, size_t n, size_t size) {
    return allocI_vec(A->I, pointer, n, size);
}

void sioI_accessor_free(morphine_sio_accessor_t A, void *pointer) {
    allocI_free(A->I, pointer);
}

void sioI_accessor_error(morphine_sio_accessor_t A, const char *str) {
    throwI_error(A->I, str);
}

void sioI_accessor_errorf(morphine_sio_accessor_t A, const char *str, ...) {
    va_list args;
    va_start(args, str);
    sioI_accessor_errorv(A, str, args);
    va_end(args);
}

void sioI_accessor_errorv(morphine_sio_accessor_t A, const char *str, va_list args) {
    struct string *result = stringI_createva(A->I, str, args);
    throwI_errorv(A->I, valueI_object(result));
}
