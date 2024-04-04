//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"

#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

stackI_ptr stack_raise(morphine_coroutine_t U, size_t size) {
    if (size == 0) {
        throwI_error(U->I, "Raise size is zero");
    }

    struct stack *stack = &U->stack;

    stackI_ptr result = stack_ptr(stack->allocated + stack->top);

    if (size > SIZE_MAX - stack->top) {
        throwI_error(U->I, "Raise top overflow");
    }

    if (stack->top + size >= stack->size) {
        size_t grow = U->stack.settings.grow;
        size_t raise_size = ((size / grow) + 1) * grow;
        size_t new_size = stack->size + raise_size;

        if (raise_size > SIZE_MAX - stack->size) {
            throwI_error(U->I, "Raise size overflow");
        }

        if (new_size >= U->stack.settings.limit) {
            throwI_error(U->I, "Stack overflow");
        }

        stack_save(stack);
        stack_ptr_save(stack->allocated, result);

        stack->allocated = allocI_uni(U->I, stack->allocated, new_size * sizeof(struct value));

        stack_ptr_recover(stack->allocated, result);
        stack_recover(stack);

        stack->size = new_size;
    }

    stack->top += size;

    for (size_t i = stack->top - size; i < stack->top; i++) {
        stack->allocated[i] = valueI_nil;
    }

    return result;
}
