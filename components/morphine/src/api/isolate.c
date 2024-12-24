//
// Created by why-iskra on 24.12.2024.
//

#include "morphine/api.h"
#include "morphine/misc/isolate.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_isolate(
    morphine_coroutine_t U,
    morphine_isolate_config_t config,
    ml_size size
) {
    struct value value = stackI_peek(U, size);
    struct value *arguments = size > 0 ? stackI_unsafe_peek(U, size - 1) : NULL;

    struct value result = isolateI_call(U->I, config, value, arguments, size);

    stackI_replace(U, size, result);
    stackI_pop(U, size);
}
