//
// Created by whyiskra on 06.01.24.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_native(morphine_state_t S, const char *name, morphine_native_t native) {
    stackI_push(S, valueI_object(nativeI_create(S->I, name, native)));
}

MORPHINE_API const char *mapi_native_name(morphine_state_t S) {
    struct value value = stackI_peek(S, 1);
    struct native *native = valueI_as_native_or_error(S, value);

    return native->name;
}

MORPHINE_API morphine_native_t mapi_native_function(morphine_state_t S) {
    struct value value = stackI_peek(S, 1);
    struct native *native = valueI_as_native_or_error(S, value);

    return native->function;
}
