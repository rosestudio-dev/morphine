//
// Created by why-iskra on 18.05.2024.
//

#pragma once

#include <stdarg.h>
#include "morphine/core/value.h"
#include "morphine/platform.h"

struct sio {
    struct object header;

    bool inited;
    void *data;

    morphine_sio_interface_t interface;
};

struct sio *sioI_create(morphine_instance_t, morphine_sio_interface_t);
void sioI_free(morphine_instance_t, struct sio *);

void sioI_open(morphine_instance_t, struct sio *, void *);
void sioI_close(morphine_instance_t, struct sio *);
size_t sioI_read(morphine_instance_t, struct sio *, uint8_t *buffer, size_t size);
size_t sioI_write(morphine_instance_t, struct sio *, const uint8_t *buffer, size_t size);
void sioI_flush(morphine_instance_t, struct sio *);

size_t sioI_print(morphine_instance_t, struct sio *, const char *);
size_t sioI_printf(morphine_instance_t, struct sio *, const char *, ...);
size_t sioI_vprintf(morphine_instance_t, struct sio *, const char *, va_list args);

void *sioI_accessor_alloc(morphine_sio_accessor_t, void *, size_t);
void *sioI_accessor_alloc_vec(morphine_sio_accessor_t, void *, size_t, size_t);
void sioI_accessor_free(morphine_sio_accessor_t, void *);
void sioI_accessor_error(morphine_sio_accessor_t, const char *);
void sioI_accessor_errorf(morphine_sio_accessor_t, const char *, ...);
void sioI_accessor_errorv(morphine_sio_accessor_t, const char *, va_list args);
