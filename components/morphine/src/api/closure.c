//
// Created by why-iskra on 06.05.2024.
//

#include "morphine/api.h"
#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_closure(morphine_coroutine_t U) {
    struct value callable = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);
    struct closure *closure = closureI_create(U->I, callable, value);
    stackI_replace(U, 1, valueI_object(closure));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_closure_lock(morphine_coroutine_t U) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 0));
    closureI_lock(U->I, closure);
}

MORPHINE_API void mapi_closure_unlock(morphine_coroutine_t U) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 0));
    closureI_unlock(U->I, closure);
}

MORPHINE_API void mapi_closure_value(morphine_coroutine_t U) {
    struct closure *closure = valueI_as_closure_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, closureI_value(U->I, closure));
}
