//
// Created by why-iskra on 19.05.2024.
//

#include "morphine/object/stream.h"
#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_push_stream_io(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->I->stream.io));
}

MORPHINE_API void mapi_push_stream_err(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->I->stream.err));
}

MORPHINE_API void mapi_push_stream(morphine_coroutine_t U, morphine_stream_interface_t interface, bool hold, void *args) {
    struct value value = valueI_nil;
    if (hold) {
        value = stackI_peek(U, 0);
    }

    struct value stream = valueI_object(streamI_create(U->I, interface, value, args));

    if (hold) {
        stackI_replace(U, 0, stream);
    } else {
        stackI_push(U, stream);
    }
}

MORPHINE_API void mapi_stream_close(morphine_coroutine_t U, bool force) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));

    if (stream == U->I->stream.io || stream == U->I->stream.err) {
        stackI_pop(U, 1);
        return;
    }

    streamI_close(U->I, stream, force);
    stackI_pop(U, 1);
}

MORPHINE_API size_t mapi_stream_read(morphine_coroutine_t U, uint8_t *buffer, size_t size) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_read(U->I, stream, buffer, size);
}

MORPHINE_API size_t mapi_stream_write(morphine_coroutine_t U, const uint8_t *buffer, size_t size) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_write(U->I, stream, buffer, size);
}

MORPHINE_API void mapi_stream_flush(morphine_coroutine_t U) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    streamI_flush(U->I, stream);
}

MORPHINE_API bool mapi_stream_seek_set(morphine_coroutine_t U, size_t offset) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_seek_set(U->I, stream, offset);
}

MORPHINE_API bool mapi_stream_seek_cur(morphine_coroutine_t U, size_t offset) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_seek_cur(U->I, stream, offset);
}

MORPHINE_API bool mapi_stream_seek_prv(morphine_coroutine_t U, size_t offset) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_seek_prv(U->I, stream, offset);
}

MORPHINE_API bool mapi_stream_seek_end(morphine_coroutine_t U, size_t offset) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_seek_end(U->I, stream, offset);
}

MORPHINE_API size_t mapi_stream_tell(morphine_coroutine_t U) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_tell(U->I, stream);
}

MORPHINE_API bool mapi_stream_eos(morphine_coroutine_t U) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_eos(U->I, stream);
}

MORPHINE_API size_t mapi_stream_print(morphine_coroutine_t U, const char *str) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));
    return streamI_print(U->I, stream, str);
}

MORPHINE_API size_t mapi_stream_printf(morphine_coroutine_t U, const char *str, ...) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 0));

    va_list args;
    va_start(args, str);
    size_t written = streamI_vprintf(U->I, stream, str, args);
    va_end(args);

    return written;
}
