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

MORPHINE_API void mapi_sio_open(morphine_coroutine_t U, void *data) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));

    if (sio == U->I->sio.io || sio == U->I->sio.error) {
        return;
    }

    sioI_open(U->I, sio, data);
}

MORPHINE_API void mapi_sio_close(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));

    if (sio == U->I->sio.io || sio == U->I->sio.error) {
        return;
    }

    sioI_close(U->I, sio);
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

MORPHINE_API void *mapi_sio_accessor_alloc(morphine_sio_accessor_t A, void *pointer, size_t size) {
    return sioI_accessor_alloc(A, pointer, size);
}

MORPHINE_API void *mapi_sio_accessor_alloc_vec(morphine_sio_accessor_t A, void *p, size_t n, size_t size) {
    return sioI_accessor_alloc_vec(A, p, n, size);
}

MORPHINE_API void mapi_sio_accessor_free(morphine_sio_accessor_t A, void *pointer) {
    sioI_accessor_free(A, pointer);
}

MORPHINE_API void mapi_sio_accessor_error(morphine_sio_accessor_t A, const char *str) {
    sioI_accessor_error(A, str);
}

MORPHINE_API void mapi_sio_accessor_errorf(morphine_sio_accessor_t A, const char *str, ...) {
    va_list args;
    va_start(args, str);
    sioI_accessor_errorv(A, str, args);
    va_end(args);
}
