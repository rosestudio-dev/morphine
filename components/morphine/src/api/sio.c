//
// Created by why-iskra on 19.05.2024.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/sio.h"

MORPHINE_API void mapi_push_sio_io(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->I->sio.io));
}

MORPHINE_API void mapi_push_sio_error(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->I->sio.error));
}

MORPHINE_API void mapi_push_sio(morphine_coroutine_t U, morphine_sio_interface_t interface) {
    stackI_push(U, valueI_object(sioI_create(U->I, interface)));
}

MORPHINE_API void mapi_sio_hold(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);

    sioI_hold(U->I, sio, value);

    stackI_pop(U, 1);
}

MORPHINE_API void mapi_sio_open(morphine_coroutine_t U, void *data) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));

    if (sio == U->I->sio.io || sio == U->I->sio.error) {
        return;
    }

    sioI_open(U->I, sio, data);
}

MORPHINE_API bool mapi_sio_is_opened(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_is_opened(U->I, sio);
}

MORPHINE_API void mapi_sio_close(morphine_coroutine_t U, bool force) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));

    if (sio == U->I->sio.io || sio == U->I->sio.error) {
        stackI_pop(U, 1);
        return;
    }

    sioI_close(U->I, sio, force);
    stackI_pop(U, 1);
}

MORPHINE_API size_t mapi_sio_read(morphine_coroutine_t U, uint8_t *buffer, size_t size) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_read(U->I, sio, buffer, size);
}

MORPHINE_API size_t mapi_sio_write(morphine_coroutine_t U, const uint8_t *buffer, size_t size) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_write(U->I, sio, buffer, size);
}

MORPHINE_API void mapi_sio_flush(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    sioI_flush(U->I, sio);
}

MORPHINE_API bool mapi_sio_seek_set(morphine_coroutine_t U, size_t offset) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_seek_set(U->I, sio, offset);
}

MORPHINE_API bool mapi_sio_seek_cur(morphine_coroutine_t U, size_t offset) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_seek_cur(U->I, sio, offset);
}

MORPHINE_API bool mapi_sio_seek_prv(morphine_coroutine_t U, size_t offset) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_seek_prv(U->I, sio, offset);
}

MORPHINE_API bool mapi_sio_seek_end(morphine_coroutine_t U, size_t offset) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_seek_end(U->I, sio, offset);
}

MORPHINE_API size_t mapi_sio_tell(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_tell(U->I, sio);
}

MORPHINE_API bool mapi_sio_eos(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_eos(U->I, sio);
}

MORPHINE_API size_t mapi_sio_print(morphine_coroutine_t U, const char *str) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    return sioI_print(U->I, sio, str);
}

MORPHINE_API size_t mapi_sio_printf(morphine_coroutine_t U, const char *str, ...) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));

    va_list args;
    va_start(args, str);
    size_t written = sioI_vprintf(U->I, sio, str, args);
    va_end(args);

    return written;
}
