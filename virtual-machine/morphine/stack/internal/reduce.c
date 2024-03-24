//
// Created by why on 3/24/24.
//

#include "functions.h"

#include "morphine/object/state.h"
#include "morphine/core/throw.h"

stackI_ptr stack_reduce(morphine_state_t S, size_t size) {
    if (size > S->stack.top) {
        throwI_message_error(S, "Cannot reduce stack");
    }

    S->stack.top -= size;

    return stack_ptr(S->stack.allocated + S->stack.top);
}