//
// Created by whyiskra on 06.01.24.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_native(morphine_coroutine_t U, morphine_native_t native) {
    struct string *name = valueI_as_string_or_error(U->I, stackI_peek(U, 0));
    stackI_replace(U, 0, valueI_object(nativeI_create(U->I, name, native)));
}

MORPHINE_API void mapi_native_name(morphine_coroutine_t U) {
    struct native *native = valueI_as_native_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(native->name));
}

MORPHINE_API morphine_native_t mapi_native_function(morphine_coroutine_t U) {
    struct native *native = valueI_as_native_or_error(U->I, stackI_peek(U, 0));
    return native->function;
}
