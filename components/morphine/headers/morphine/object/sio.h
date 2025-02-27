//
// Created by why-iskra on 18.05.2024.
//

#pragma once

#include <stdarg.h>
#include "morphine/core/value.h"
#include "morphine/platform.h"

struct sio {
    struct object header;

    bool opened;
    void *data;
    struct value hold_value;

    morphine_sio_interface_t interface;
};

struct sio *sioI_create(morphine_instance_t, morphine_sio_interface_t);
void sioI_free(morphine_instance_t, struct sio *);

void sioI_hold(morphine_instance_t, struct sio *, struct value);

void sioI_open(morphine_instance_t, struct sio *, void *);
bool sioI_is_opened(morphine_instance_t, struct sio *);
void sioI_close(morphine_instance_t, struct sio *, bool force);
size_t sioI_read(morphine_instance_t, struct sio *, uint8_t *buffer, size_t size);
size_t sioI_write(morphine_instance_t, struct sio *, const uint8_t *buffer, size_t size);
void sioI_flush(morphine_instance_t, struct sio *);
bool sioI_seek_set(morphine_instance_t, struct sio *, size_t);
bool sioI_seek_cur(morphine_instance_t, struct sio *, size_t);
bool sioI_seek_prv(morphine_instance_t, struct sio *, size_t);
bool sioI_seek_end(morphine_instance_t, struct sio *, size_t);
size_t sioI_tell(morphine_instance_t, struct sio *);
bool sioI_eos(morphine_instance_t, struct sio *);

size_t sioI_print(morphine_instance_t, struct sio *, const char *);
morphine_printf(3, 4) size_t sioI_printf(morphine_instance_t, struct sio *, const char *, ...);
size_t sioI_vprintf(morphine_instance_t, struct sio *, const char *, va_list args);
