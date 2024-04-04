//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"

#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

stackI_ptr stack_reduce(morphine_coroutine_t U, size_t size) {
    if (size > U->stack.top) {
        throwI_error(U->I, "Cannot reduce stack");
    }

    U->stack.top -= size;

    return stack_ptr(U->stack.allocated + U->stack.top);
}