//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_rotate(morphine_coroutine_t U, size_t count) {
    stackI_rotate(U, count);
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

MORPHINE_API void mapi_stack_reset(morphine_coroutine_t U) {
    stackI_pop(U, stackI_space(U));
}

MORPHINE_API size_t mapi_stack_used(morphine_coroutine_t U) {
    return stackI_space(U);
}

MORPHINE_API void mapi_stack_set_limit(morphine_coroutine_t U, size_t limit) {
    stackI_set_limit(U, limit);
}
