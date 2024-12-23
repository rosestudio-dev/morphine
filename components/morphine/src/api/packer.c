//
// Created by why-iskra on 21.08.2024.
//

#include "morphine/api.h"
#include "morphine/core/value.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc/packer.h"

MORPHINE_API void mapi_pack(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    packerI_to(U->I, sio, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_unpack(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    struct value value = packerI_from(U->I, sio);
    stackI_push(U, value);
}
