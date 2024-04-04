//
// Created by whyiskra on 06.01.24.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine/stack/access.h"

MORPHINE_API void mapi_push_native(morphine_coroutine_t U, const char *name, morphine_native_t native) {
    stackI_push(U, valueI_object(nativeI_create(U->I, name, native)));
}

MORPHINE_API const char *mapi_native_name(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 1);
    struct native *native = valueI_as_native_or_error(U->I, value);

    return native->name;
}

MORPHINE_API morphine_native_t mapi_native_function(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 1);
    struct native *native = valueI_as_native_or_error(U->I, value);

    return native->function;
}
