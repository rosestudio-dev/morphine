//
// Created by why on 2/8/25.
//

#include "morphine/auxiliary/vector.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_vector_clear(morphine_coroutine_t U) {
    mapi_vector_resize(U, 0);
}

MORPHINE_AUX void maux_vector_push(morphine_coroutine_t U) {
    mapi_peek(U, 1);
    ml_size size = mapi_vector_len(U);
    mapi_pop(U, 1);

    mapi_vector_add(U, size);
}

MORPHINE_AUX void maux_vector_pop(morphine_coroutine_t U) {
    ml_size size = mapi_vector_len(U);
    mapi_vector_remove(U, size - 1);
}

MORPHINE_AUX void maux_vector_peek(morphine_coroutine_t U) {
    ml_size size = mapi_vector_len(U);
    mapi_vector_get(U, size - 1);
}

MORPHINE_AUX void maux_vector_push_front(morphine_coroutine_t U) {
    mapi_vector_add(U, 0);
}

MORPHINE_AUX void maux_vector_pop_front(morphine_coroutine_t U) {
    mapi_vector_remove(U, 0);
}

MORPHINE_AUX void maux_vector_peek_front(morphine_coroutine_t U) {
    mapi_vector_get(U, 0);
}

