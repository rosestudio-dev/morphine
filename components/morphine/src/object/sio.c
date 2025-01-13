//
// Created by why-iskra on 18.05.2024.
//

#include "morphine/object/sio.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/string.h"
#include <string.h>

struct sio *sioI_create(morphine_instance_t I, morphine_sio_interface_t interface, void *args) {
    if (interface.write == NULL && interface.read == NULL) {
        throwI_error(I, "sio interface hasn't read/write functions");
    }

    gcI_safe_enter(I);

    // create
    struct sio *result = allocI_uni(I, NULL, sizeof(struct sio));
    (*result) = (struct sio) {
        .interface = interface,
        .opened = false,
        .data = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_SIO);

    // config
    gcI_safe(I, valueI_object(result));

    result->data = allocI_uni(I, NULL, interface.data_size);
    if (interface.open != NULL) {
        interface.open(I, result->data, args);
    }

    result->opened = true;

    gcI_safe_exit(I);

    return result;
}

static inline void close(morphine_instance_t I, struct sio *sio, bool force) {
    if (!force && !sio->opened) {
        throwI_error(I, "sio already closed");
    }

    if (sio->opened) {
        sio->opened = false;

        if (sio->interface.close != NULL) {
            sio->interface.close(I, sio->data);
        }
    }
}

void sioI_free(morphine_instance_t I, struct sio *sio) {
    close(I, sio, true);
    allocI_free(I, sio->data);
    allocI_free(I, sio);
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

    return sio->interface.read(I, sio->data, buffer, size);
}

size_t sioI_write(morphine_instance_t I, struct sio *sio, const uint8_t *buffer, size_t size) {
    checks(I, sio);

    if (sio->interface.write == NULL) {
        throwI_error(I, "sio is read only");
    }

    return sio->interface.write(I, sio->data, buffer, size);
}

void sioI_flush(morphine_instance_t I, struct sio *sio) {
    checks(I, sio);

    if (sio->interface.flush != NULL) {
        return sio->interface.flush(I, sio->data);
    }
}

static inline bool sio_seek(morphine_instance_t I, struct sio *sio, size_t offset, morphine_sio_seek_mode_t mode) {
    checks(I, sio);

    if (sio->interface.seek == NULL) {
        return false;
    }

    return sio->interface.seek(I, sio->data, offset, mode);
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

    return sio->interface.tell(I, sio->data);
}

bool sioI_eos(morphine_instance_t I, struct sio *sio) {
    checks(I, sio);

    if (sio->interface.eos == NULL) {
        return false;
    }

    return sio->interface.eos(I, sio->data);
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
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(sio));
    struct string *result = gcI_safe_obj(I, string, stringI_createva(I, str, args));
    size_t written = sioI_write(I, sio, (const uint8_t *) result->chars, result->size);
    gcI_safe_exit(I);

    return written;
}
