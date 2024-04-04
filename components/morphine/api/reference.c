//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/reference.h"

MORPHINE_API void mapi_push_ref(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);
    *value = valueI_object(referenceI_create(U->I, *value));
}

MORPHINE_API void mapi_ref_set(morphine_coroutine_t U) {
    struct value reference = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    referenceI_set(U->I, valueI_as_reference_or_error(U->I, reference), value);

    stackI_pop(U, 1);
}

MORPHINE_API void mapi_ref_get(morphine_coroutine_t U) {
    struct value reference = stackI_peek(U, 0);

    stackI_push(U, valueI_nil);
    struct value *value = stackI_vector(U, 0, 1);

    *value = *referenceI_get(U->I, valueI_as_reference_or_error(U->I, reference));
}
