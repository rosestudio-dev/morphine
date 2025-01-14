//
// Created by why-iskra on 18.05.2024.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/platform.h"
#include <stdarg.h>

struct stream {
    struct object header;
    morphine_stream_interface_t interface;
    bool opened;
    void *data;
};

struct stream *streamI_create(morphine_instance_t, morphine_stream_interface_t, void *args);
void streamI_free(morphine_instance_t, struct stream *);

void streamI_close(morphine_instance_t, struct stream *, bool force);
size_t streamI_read(morphine_instance_t, struct stream *, uint8_t *buffer, size_t size);
size_t streamI_write(morphine_instance_t, struct stream *, const uint8_t *buffer, size_t size);
void streamI_flush(morphine_instance_t, struct stream *);
bool streamI_seek_set(morphine_instance_t, struct stream *, size_t);
bool streamI_seek_cur(morphine_instance_t, struct stream *, size_t);
bool streamI_seek_prv(morphine_instance_t, struct stream *, size_t);
bool streamI_seek_end(morphine_instance_t, struct stream *, size_t);
size_t streamI_tell(morphine_instance_t, struct stream *);
bool streamI_eos(morphine_instance_t, struct stream *);

size_t streamI_print(morphine_instance_t, struct stream *, const char *);
morphine_printf(3, 4) size_t streamI_printf(morphine_instance_t, struct stream *, const char *, ...);
size_t streamI_vprintf(morphine_instance_t, struct stream *, const char *, va_list args);
