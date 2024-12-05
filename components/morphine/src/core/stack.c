//
// Created by why-iskra on 04.04.2024.
//

#include "morphine/core/stack.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/object/coroutine.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/utils/overflow.h"
#include "morphine/params.h"

static inline void callstack_recover(struct coroutine *U, struct value *old_stack, size_t old_top) {
    struct callinfo *current = U->callstack.callinfo;
    while (current != NULL) {
#define stackI_stack_ptr_recover(ptr) do { (ptr) = U->stack.array.allocated + ((size_t) ((ptr) - old_stack)); } while(0)
#define stackI_ptr_recover(ptr) do { if(ptr >= old_stack && ptr < (old_stack + old_top)) { stackI_stack_ptr_recover(ptr); } } while(0)
        stackI_stack_ptr_recover(current->s.stack.base);
        stackI_stack_ptr_recover(current->s.stack.space);
        stackI_stack_ptr_recover(current->s.stack.top);
        stackI_ptr_recover(current->s.direct.callable);
        stackI_ptr_recover(current->s.direct.env);
        stackI_ptr_recover(current->s.direct.self);
        stackI_ptr_recover(current->s.direct.args);
#undef stackI_stack_ptr_recover
#undef stackI_ptr_recover

        current = current->prev;
    }
}

static inline size_t stack_space(morphine_coroutine_t U, struct callinfo *callinfo) {
    if (callinfo == NULL) {
        return U->stack.space_top;
    }

    return (size_t) (callinfo->s.stack.top - callinfo->s.stack.space);
}

static inline struct value *stack_peek(morphine_coroutine_t U, struct callinfo *callinfo, size_t offset) {
    size_t space_size = stack_space(U, callinfo);

    struct value *p;
    if (callinfo == NULL) {
        p = U->stack.array.allocated + U->stack.space_top;
    } else {
        p = callinfo->s.stack.top;
    }

    if (offset >= space_size) {
        throwI_error(U->I, "cannot peek value from stack");
    }

    return p - offset - 1;
}

static struct value *stack_vector(
    morphine_coroutine_t U,
    struct callinfo *callinfo,
    size_t offset,
    size_t size
) {
    if (size == 0) {
        return NULL;
    }

    size_t space_size = stack_space(U, callinfo);

    struct value *p;
    if (callinfo == NULL) {
        p = U->stack.array.allocated + U->stack.space_top;
    } else {
        p = callinfo->s.stack.top;
    }

    if (offset + size > space_size) {
        throwI_error(U->I, "cannot peek vector from stack");
    }

    return (p - offset - size);
}

struct stack stackI_prototype(morphine_instance_t I) {
    return (struct stack) {
        .array.allocated = NULL,
        .array.size = 0,
        .array.top = 0,
        .space_top = 0,
        .settings.limit = I->settings.coroutines.stack.limit,
        .settings.allow_shrinking = true,
    };
}

void stackI_destruct(morphine_instance_t I, struct stack *stack) {
    allocI_free(I, stack->array.allocated);
}

void stackI_throw_fix(morphine_coroutine_t U) {
    U->stack.settings.allow_shrinking = true;
}

struct value *stackI_raise(morphine_coroutine_t U, size_t size) {
    if (size == 0) {
        throwI_error(U->I, "raise size is zero");
    }

    morphine_instance_t I = U->I;
    struct stack *stack = &U->stack;

    overflow_add(size, stack->array.top, SIZE_MAX) {
        throwI_error(I, "stack overflow");
    }

    if (stack->array.top + size >= stack->array.size) {
        size_t grow = MPARAM_STACK_GROW;
        if (grow < size) {
            grow = size;
        }

        size_t new_size = stack->array.size + grow;

        if (overflow_condition_add(grow, stack->array.size, SIZE_MAX) || new_size >= stack->settings.limit) {
            throwI_error(I, "stack overflow");
        }

        struct value *old_allocated = stack->array.allocated;
        size_t old_top = stack->array.top;

        U->stack.settings.allow_shrinking = false;
        stack->array.allocated = allocI_vec(I, stack->array.allocated, new_size, sizeof(struct value));
        U->stack.settings.allow_shrinking = true;

        if (stack->array.allocated != old_allocated) {
            callstack_recover(U, old_allocated, old_top);
        }

        stack->array.size = new_size;
    }

    stack->array.top += size;

    for (size_t i = stack->array.top - size; i < stack->array.top; i++) {
        stack->array.allocated[i] = valueI_nil;
    }

    return stack->array.allocated + (stack->array.top - size);
}

struct value *stackI_reduce(morphine_coroutine_t U, size_t size) {
    if (size > U->stack.array.top) {
        throwI_error(U->I, "cannot reduce stack");
    }

    U->stack.array.top -= size;

    return U->stack.array.allocated + U->stack.array.top;
}

void stackI_shrink(morphine_coroutine_t U) {
    size_t size = U->stack.array.top + MPARAM_STACK_GROW;

    if (!U->stack.settings.allow_shrinking ||
        U->stack.array.size <= size) {
        return;
    }

    U->stack.settings.allow_shrinking = false;

    struct value *saved_allocated = U->stack.array.allocated;
    size_t saved_top = U->stack.array.top;

    U->stack.array.allocated = allocI_vec(
        U->I, U->stack.array.allocated, size, sizeof(struct value)
    );

    if (U->stack.array.allocated != saved_allocated) {
        callstack_recover(U, saved_allocated, saved_top);
    }

    U->stack.array.size = size;

    U->stack.settings.allow_shrinking = true;
}

size_t stackI_space(morphine_coroutine_t U) {
    return stack_space(U, callstackI_info(U));
}

void stackI_push(morphine_coroutine_t U, struct value value) {
    gcI_safe_enter(U->I);
    gcI_safe(U->I, value);

    stackI_raise(U, 1);
    if (callstackI_info(U) != NULL) {
        callstackI_info(U)->s.stack.top++;
    } else {
        U->stack.space_top++;
    }

    struct value *stack_value = U->stack.array.allocated + U->stack.array.top - 1;
    (*stack_value) = value;

    gcI_safe_exit(U->I);
}

struct value stackI_peek(morphine_coroutine_t U, size_t offset) {
    return *stack_peek(U, callstackI_info(U), offset);
}

void stackI_pop(morphine_coroutine_t U, size_t count) {
    if (count == 0) {
        return;
    }

    struct callinfo *callinfo = callstackI_info(U);
    size_t size = stack_space(U, callinfo);

    if (count > size) {
        throwI_error(U->I, "cannot pop from stack");
    }

    if (callinfo == NULL) {
        U->stack.space_top -= count;
        stackI_reduce(U, count);
    } else {
        callinfo->s.stack.top = stackI_reduce(U, count);
    }
}

void stackI_rotate(morphine_coroutine_t U, size_t count) {
    if (count == 0) {
        return;
    }

    if (stackI_space(U) == 0) {
        throwI_error(U->I, "cannot rotate empty stack");
    }

    struct value *values = stack_vector(U, callstackI_info(U), 0, count);

    struct value temp = values[count - 1];
    for (size_t i = 0; i < count - 1; i++) {
        values[count - i - 1] = values[count - i - 2];
    }
    values[0] = temp;
}

void stackI_replace(morphine_coroutine_t U, size_t offset, struct value value) {
    *stack_peek(U, callstackI_info(U), offset) = value;
}

struct value *stackI_unsafe_peek(morphine_coroutine_t U, size_t offset) {
    return stack_peek(U, callstackI_info(U), offset);
}

void stackI_set_limit(morphine_coroutine_t U, size_t limit) {
    U->stack.settings.limit = limit;
}
