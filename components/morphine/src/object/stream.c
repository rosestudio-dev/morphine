//
// Created by why-iskra on 18.05.2024.
//

#include "morphine/object/stream.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/string.h"
#include <string.h>

struct stream *streamI_create(morphine_instance_t I, morphine_stream_interface_t interface, void *args) {
    if (interface.write == NULL && interface.read == NULL) {
        throwI_error(I, "stream interface hasn't read/write functions");
    }

    gcI_safe_enter(I);

    // create
    struct stream *result = allocI_uni(I, NULL, sizeof(struct stream));
    (*result) = (struct stream) {
        .interface = interface,
        .opened = false,
        .data = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_STREAM);

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

static inline void close(morphine_instance_t I, struct stream *stream, bool force) {
    if (!force && !stream->opened) {
        throwI_error(I, "stream already closed");
    }

    if (stream->opened) {
        stream->opened = false;

        if (stream->interface.close != NULL) {
            stream->interface.close(I, stream->data);
        }
    }
}

void streamI_free(morphine_instance_t I, struct stream *stream) {
    close(I, stream, true);
    allocI_free(I, stream->data);
    allocI_free(I, stream);
}

void streamI_close(morphine_instance_t I, struct stream *stream, bool force) {
    if (stream == NULL) {
        throwI_error(I, "stream is null");
    }

    close(I, stream, force);
}

static void checks(morphine_instance_t I, struct stream *stream) {
    if (stream == NULL) {
        throwI_error(I, "stream is null");
    }

    if (!stream->opened) {
        throwI_error(I, "stream isn't opened");
    }
}

size_t streamI_read(morphine_instance_t I, struct stream *stream, uint8_t *buffer, size_t size) {
    checks(I, stream);

    if (stream->interface.read == NULL) {
        throwI_error(I, "stream is write only");
    }

    return stream->interface.read(I, stream->data, buffer, size);
}

size_t streamI_write(morphine_instance_t I, struct stream *stream, const uint8_t *buffer, size_t size) {
    checks(I, stream);

    if (stream->interface.write == NULL) {
        throwI_error(I, "stream is read only");
    }

    return stream->interface.write(I, stream->data, buffer, size);
}

void streamI_flush(morphine_instance_t I, struct stream *stream) {
    checks(I, stream);

    if (stream->interface.flush != NULL) {
        return stream->interface.flush(I, stream->data);
    }
}

static inline bool stream_seek(
    morphine_instance_t I,
    struct stream *stream,
    size_t offset,
    morphine_stream_seek_mode_t mode
) {
    checks(I, stream);

    if (stream->interface.seek == NULL) {
        return false;
    }

    return stream->interface.seek(I, stream->data, offset, mode);
}

bool streamI_seek_set(morphine_instance_t I, struct stream *stream, size_t offset) {
    return stream_seek(I, stream, offset, MORPHINE_STREAM_SEEK_MODE_SET);
}

bool streamI_seek_cur(morphine_instance_t I, struct stream *stream, size_t offset) {
    return stream_seek(I, stream, offset, MORPHINE_STREAM_SEEK_MODE_CUR);
}

bool streamI_seek_prv(morphine_instance_t I, struct stream *stream, size_t offset) {
    return stream_seek(I, stream, offset, MORPHINE_STREAM_SEEK_MODE_PRV);
}

bool streamI_seek_end(morphine_instance_t I, struct stream *stream, size_t offset) {
    return stream_seek(I, stream, offset, MORPHINE_STREAM_SEEK_MODE_END);
}

size_t streamI_tell(morphine_instance_t I, struct stream *stream) {
    checks(I, stream);

    if (stream->interface.tell == NULL) {
        return 0;
    }

    return stream->interface.tell(I, stream->data);
}

bool streamI_eos(morphine_instance_t I, struct stream *stream) {
    checks(I, stream);

    if (stream->interface.eos == NULL) {
        return false;
    }

    return stream->interface.eos(I, stream->data);
}

size_t streamI_print(morphine_instance_t I, struct stream *stream, const char *str) {
    return streamI_write(I, stream, (const uint8_t *) str, strlen(str));
}

size_t streamI_printf(morphine_instance_t I, struct stream *stream, const char *str, ...) {
    va_list args;
    va_start(args, str);
    size_t written = streamI_vprintf(I, stream, str, args);
    va_end(args);

    return written;
}

size_t streamI_vprintf(morphine_instance_t I, struct stream *stream, const char *str, va_list args) {
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(stream));
    struct string *result = gcI_safe_obj(I, string, stringI_createva(I, str, args));
    size_t written = streamI_write(I, stream, (const uint8_t *) result->chars, result->size);
    gcI_safe_exit(I);

    return written;
}
