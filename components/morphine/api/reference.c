//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/state.h"
#include "morphine/object/reference.h"
#include "morphine/stack/access.h"

MORPHINE_API void mapi_push_ref(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_object(referenceI_create(S->I, *value));
}

MORPHINE_API void mapi_ref_set(morphine_state_t S) {
    struct value reference = stackI_peek(S, 1);
    struct value value = stackI_peek(S, 0);

    referenceI_set(S->I, valueI_as_reference_or_error(S->I, reference), value);

    stackI_pop(S, 1);
}

MORPHINE_API void mapi_ref_get(morphine_state_t S) {
    struct value reference = stackI_peek(S, 0);

    stackI_push(S, valueI_nil);
    struct value *value = stackI_vector(S, 0, 1);

    *value = *referenceI_get(S->I, valueI_as_reference_or_error(S->I, reference));
}
