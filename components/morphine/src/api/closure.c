//
// Created by why-iskra on 06.05.2024.
//

#include "morphine/api.h"
#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_closure(morphine_coroutine_t U, ml_size size) {
    stackI_push(U, valueI_object(closureI_create(U->I, stackI_peek(U, 0), size)));
}

MORPHINE_API void mapi_closure_get(morphine_coroutine_t U, ml_size index) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 0));
    struct value value = closureI_get(U->I, closure, index);
    stackI_push(U, value);
}

MORPHINE_API void mapi_closure_set(morphine_coroutine_t U, ml_size index) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 1));
    closureI_set(U->I, closure, index, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API ml_size mapi_closure_size(morphine_coroutine_t U) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 0));
    return closure->size;
}
