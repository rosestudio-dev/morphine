//
// Created by whyiskra on 3/24/24.
//

#include "morphine/object/coroutine/stack/access.h"
#include "morphine/object/coroutine/stack/call.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "functions.h"

size_t stackI_space_size(morphine_coroutine_t U) {
    return stack_space_size(U, callstackI_info(U));
}

void stackI_push(morphine_coroutine_t U, struct value value) {
    gcI_safe(U->I, value);
    if (callstackI_info(U) == NULL) {
        stack_raise(U, 1);
    } else {
        stack_raise(U, 1);
        callstackI_info(U)->s.top.p++;
    }

    struct value *stack_value = U->stack.allocated + U->stack.top - 1;
    (*stack_value) = value;
    gcI_reset_safe(U->I);
}

struct value stackI_peek(morphine_coroutine_t U, size_t offset) {
    return stack_peek(U, callstackI_info(U), offset);
}

struct value *stackI_vector(morphine_coroutine_t U, size_t offset, size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct value *p;
    size_t space_size;

    if (callstackI_info(U) == NULL) {
        space_size = U->stack.top;
        p = U->stack.allocated + U->stack.top;
    } else {
        space_size = stackI_space_size(U);
        p = callstackI_info(U)->s.top.p;
    }

    if (offset + size > space_size) {
        throwI_error(U->I, "Cannot peek vector from space");
    }

    return (p - offset - size);
}

void stackI_pop(morphine_coroutine_t U, size_t count) {
    if (count == 0) {
        return;
    }

    if (callstackI_info(U) == NULL) {
        stack_reduce(U, count);
    } else {
        struct callinfo *callinfo = callstackI_info(U);
        size_t size = stackI_space_size(U);

        if (count > size) {
            throwI_error(U->I, "Cannot pop from space");
        }

        callinfo->s.top = stack_reduce(U, count);
    }
}