//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/reference.h"

MORPHINE_API void mapi_push_ref(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_object(referenceI_create(U->I, value));
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_ref_set(morphine_coroutine_t U) {
    struct value reference = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    referenceI_set(valueI_as_reference_or_error(U->I, reference), value);

    stackI_pop(U, 1);
}

MORPHINE_API void mapi_ref_get(morphine_coroutine_t U) {
    struct value reference = stackI_peek(U, 0);

    stackI_push(U, valueI_nil);
    struct value result = *referenceI_get(valueI_as_reference_or_error(U->I, reference));
    stackI_replace(U, 0, result);
}
