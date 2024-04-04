//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/coroutine/stack/access.h"

MORPHINE_API void mapi_rotate(morphine_coroutine_t U, size_t count) {
    if (count == 0) {
        return;
    }

    struct value *values = stackI_vector(U, 0, count);

    struct value temp = values[count - 1];
    for (size_t i = 0; i < count - 1; i++) {
        values[count - i - 1] = values[count - i - 2];
    }
    values[0] = temp;
}

MORPHINE_API void mapi_pop(morphine_coroutine_t U, size_t size) {
    stackI_pop(U, size);
}

MORPHINE_API void mapi_peek(morphine_coroutine_t U, size_t offset) {
    stackI_push(U, stackI_peek(U, offset));
}

MORPHINE_API void mapi_move(morphine_coroutine_t U, morphine_coroutine_t to) {
    stackI_push(to, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_copy(morphine_coroutine_t U, morphine_coroutine_t to, size_t offset) {
    stackI_push(to, stackI_peek(U, offset));
}

MORPHINE_API size_t mapi_stack_size(morphine_coroutine_t U) {
    return U->stack.top;
}

MORPHINE_API void mapi_stack_reset(morphine_coroutine_t U) {
    stackI_pop(U, stackI_space_size(U));
}
