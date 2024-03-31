//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/stack/access.h"

MORPHINE_API void mapi_rotate(morphine_state_t S, size_t count) {
    if (count == 0) {
        return;
    }

    struct value *values = stackI_vector(S, 0, count);

    struct value temp = values[count - 1];
    for (size_t i = 0; i < count - 1; i++) {
        values[count - i - 1] = values[count - i - 2];
    }
    values[0] = temp;
}

MORPHINE_API void mapi_pop(morphine_state_t S, size_t size) {
    stackI_pop(S, size);
}

MORPHINE_API void mapi_peek(morphine_state_t S, size_t offset) {
    stackI_push(S, stackI_peek(S, offset));
}

MORPHINE_API void mapi_move(morphine_state_t S, morphine_state_t to) {
    stackI_push(to, stackI_peek(S, 0));
    stackI_pop(S, 1);
}

MORPHINE_API void mapi_copy(morphine_state_t S, morphine_state_t to, size_t offset) {
    stackI_push(to, stackI_peek(S, offset));
}

MORPHINE_API size_t mapi_stack_size(morphine_state_t S) {
    return S->stack.top;
}

MORPHINE_API void mapi_stack_reset(morphine_state_t S) {
    stackI_pop(S, stackI_space_size(S));
}
