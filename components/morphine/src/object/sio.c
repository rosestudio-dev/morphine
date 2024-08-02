//
// Created by why-iskra on 18.05.2024.
//

#include <string.h>
#include "morphine/object/sio.h"
#include "morphine/core/throw.h"
#include "morphine/object/string.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/gc/barrier.h"

struct sio_accessor {
    morphine_instance_t I;
};

struct sio *sioI_create(morphine_instance_t I, morphine_sio_interface_t interface) {
    if (interface.write == NULL && interface.read == NULL) {
        throwI_error(I, "sio interface hasn't read/write functions");
    }

    struct sio *result = allocI_uni(I, NULL, sizeof(struct sio));

    (*result) = (struct sio) {
        .interface = interface,
        .opened = false,
        .data = NULL,
        .hold_value = valueI_nil
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_SIO);

    return result;
}

static inline struct sio_accessor get_accessor(morphine_instance_t I) {
    return (struct sio_accessor) { .I = I };
}

static inline void close(morphine_instance_t I, struct sio *sio, bool force) {
    if (!force && !sio->opened) {
        throwI_error(I, "sio already closed");
    }

    if (sio->opened && sio->interface.close != NULL) {
        sio->opened = false;
        struct sio_accessor A = get_accessor(I);
        sio->interface.close(&A, sio->data);
    }
}

void sioI_free(morphine_instance_t I, struct sio *sio) {
    close(I, sio, true);
    allocI_free(I, sio);
}

void sioI_hold(morphine_instance_t I, struct sio *sio, struct value value) {
    if (sio == NULL) {
        throwI_error(I, "sio is null");
    }

    if (!valueI_is_nil(sio->hold_value)) {
        throwI_error(I, "sio is already holding value");
    }

    gcI_barrier(I, sio, value);
    sio->hold_value = value;
}

void sioI_open(morphine_instance_t I, struct sio *sio, void *arg) {
    if (sio == NULL) {
        throwI_error(I, "sio is null");
    }

    if (sio->opened) {
        throwI_error(I, "sio is already opened");
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
        throwI_error(I, "sio is null");
    }

    return sio->opened;
}

void sioI_close(morphine_instance_t I, struct sio *sio, bool force) {
    if (sio == NULL) {
        throwI_error(I, "sio is null");
    }

    close(I, sio, force);
}

static void checks(morphine_instance_t I, struct sio *sio) {
    if (sio == NULL) {
        throwI_error(I, "sio is null");
    }

    if (!sio->opened) {
        throwI_error(I, "sio isn't opened");
    }
}

size_t sioI_read(morphine_instance_t I, struct sio *sio, uint8_t *buffer, size_t size) {
    checks(I, sio);

    if (sio->interface.read == NULL) {
        throwI_error(I, "sio is write only");
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.read(&A, sio->data, buffer, size);
}

size_t sioI_write(morphine_instance_t I, struct sio *sio, const uint8_t *buffer, size_t size) {
    checks(I, sio);

    if (sio->interface.write == NULL) {
        throwI_error(I, "sio is read only");
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.write(&A, sio->data, buffer, size);
}

void sioI_flush(morphine_instance_t I, struct sio *sio) {
    checks(I, sio);

    if (sio->interface.flush != NULL) {
        struct sio_accessor A = get_accessor(I);
        return sio->interface.flush(&A, sio->data);
    }
}

static inline bool sio_seek(
    morphine_instance_t I,
    struct sio *sio,
    size_t offset,
    morphine_sio_seek_mode_t mode
) {
    checks(I, sio);

    if (sio->interface.seek == NULL) {
        return false;
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.seek(&A, sio->data, offset, mode);
}

bool sioI_seek_set(morphine_instance_t I, struct sio *sio, size_t offset) {
    return sio_seek(I, sio, offset, MORPHINE_SIO_SEEK_MODE_SET);
}

bool sioI_seek_cur(morphine_instance_t I, struct sio *sio, size_t offset) {
    return sio_seek(I, sio, offset, MORPHINE_SIO_SEEK_MODE_CUR);
}

bool sioI_seek_prv(morphine_instance_t I, struct sio *sio, size_t offset) {
    return sio_seek(I, sio, offset, MORPHINE_SIO_SEEK_MODE_PRV);
}

bool sioI_seek_end(morphine_instance_t I, struct sio *sio, size_t offset) {
    return sio_seek(I, sio, offset, MORPHINE_SIO_SEEK_MODE_END);
}

size_t sioI_tell(morphine_instance_t I, struct sio *sio) {
    checks(I, sio);

    if (sio->interface.tell == NULL) {
        return 0;
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.tell(&A, sio->data);
}

bool sioI_eos(morphine_instance_t I, struct sio *sio) {
    checks(I, sio);

    if (sio->interface.eos == NULL) {
        return false;
    }

    struct sio_accessor A = get_accessor(I);
    return sio->interface.eos(&A, sio->data);
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
