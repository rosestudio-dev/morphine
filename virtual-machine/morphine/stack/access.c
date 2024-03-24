//
// Created by why on 3/24/24.
//

#include "morphine/stack/access.h"
#include "morphine/stack/call.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "internal/functions.h"

size_t stackI_space_size(morphine_state_t S) {
    return stack_space_size(S, callstackI_info(S));
}

void stackI_push(morphine_state_t S, struct value value) {
    if (callstackI_info(S) == NULL) {
        stack_raise(S, value, 1);
    } else {
        stack_raise(S, value, 1);
        callstackI_info(S)->s.top.p++;
    }
}

struct value stackI_peek(morphine_state_t S, size_t offset) {
    return stack_peek(S, callstackI_info(S), offset);
}

struct value *stackI_vector(morphine_state_t S, size_t offset, size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct value *p;
    size_t space_size;

    if (callstackI_info(S) == NULL) {
        space_size = S->stack.top;
        p = S->stack.allocated + S->stack.top;
    } else {
        space_size = stackI_space_size(S);
        p = callstackI_info(S)->s.top.p;
    }

    if (offset + size > space_size) {
        throwI_message_error(S, "Cannot peek vector from space");
    }

    return (p - offset - size);
}

void stackI_pop(morphine_state_t S, size_t count) {
    if (count == 0) {
        return;
    }

    if (callstackI_info(S) == NULL) {
        stack_reduce(S, count);
    } else {
        struct callinfo *callinfo = callstackI_info(S);
        size_t size = stackI_space_size(S);

        if (count > size) {
            throwI_message_error(S, "Cannot pop from space");
        }

        callinfo->s.top = stack_reduce(S, count);
    }
}