//
// Created by why-iskra on 04.04.2024.
//

#include "morphine/object/coroutine/stack.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"

static inline void callstack_save(struct coroutine *U) {
    struct callinfo *current = U->callstack.callinfo;
    while (current != NULL) {
        stackI_ptr_save(U->stack.allocated, current->s.callable);
        stackI_ptr_save(U->stack.allocated, current->s.source);
        stackI_ptr_save(U->stack.allocated, current->s.env);
        stackI_ptr_save(U->stack.allocated, current->s.self);
        stackI_ptr_save(U->stack.allocated, current->s.result);
        stackI_ptr_save(U->stack.allocated, current->s.thrown);
        stackI_ptr_save(U->stack.allocated, current->s.args);
        stackI_ptr_save(U->stack.allocated, current->s.slots);
        stackI_ptr_save(U->stack.allocated, current->s.params);
        stackI_ptr_save(U->stack.allocated, current->s.space);
        stackI_ptr_save(U->stack.allocated, current->s.top);

        current = current->prev;
    }
}

static inline void callstack_recover(struct coroutine *U) {
    struct callinfo *current = U->callstack.callinfo;
    while (current != NULL) {
        stackI_ptr_recover(U->stack.allocated, current->s.callable);
        stackI_ptr_recover(U->stack.allocated, current->s.source);
        stackI_ptr_recover(U->stack.allocated, current->s.env);
        stackI_ptr_recover(U->stack.allocated, current->s.self);
        stackI_ptr_recover(U->stack.allocated, current->s.result);
        stackI_ptr_recover(U->stack.allocated, current->s.thrown);
        stackI_ptr_recover(U->stack.allocated, current->s.args);
        stackI_ptr_recover(U->stack.allocated, current->s.slots);
        stackI_ptr_recover(U->stack.allocated, current->s.params);
        stackI_ptr_recover(U->stack.allocated, current->s.space);
        stackI_ptr_recover(U->stack.allocated, current->s.top);

        current = current->prev;
    }
}

struct stack stackI_prototype(morphine_instance_t I, size_t limit, size_t grow) {
    if (grow == 0) {
        throwI_panic(I, "Stack grow size is zero");
    }

    if (limit == 0) {
        throwI_panic(I, "Stack limit is zero");
    }

    return (struct stack) {
        .allocated = NULL,
        .size = 0,
        .top = 0,
        .settings.grow = grow,
        .settings.limit = limit,
    };
}

void stackI_destruct(morphine_instance_t I, struct stack *stack) {
    allocI_free(I, stack->allocated);
}

struct value *stackI_raise(morphine_coroutine_t U, size_t size) {
    if (size == 0) {
        throwI_error(U->I, "Raise size is zero");
    }

    struct stack *stack = &U->stack;

    stackI_ptr result = stackI_ptr(stack->allocated + stack->top);

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

        callstack_save(U);
        stackI_ptr_save(stack->allocated, result);

        stack->allocated = allocI_uni(U->I, stack->allocated, new_size * sizeof(struct value));

        stackI_ptr_recover(stack->allocated, result);
        callstack_recover(U);

        stack->size = new_size;
    }

    stack->top += size;

    for (size_t i = stack->top - size; i < stack->top; i++) {
        stack->allocated[i] = valueI_nil;
    }

    return result.p;
}

struct value *stackI_reduce(morphine_coroutine_t U, size_t size) {
    if (size > U->stack.top) {
        throwI_error(U->I, "Cannot reduce stack");
    }

    U->stack.top -= size;

    return U->stack.allocated + U->stack.top;
}

void stackI_shrink(morphine_coroutine_t U) {
    size_t grow = U->stack.settings.grow;

    if (U->stack.top + grow >= U->stack.size) {
        return;
    }

    size_t size = U->stack.top + grow;

    callstack_save(U);

    U->stack.allocated = allocI_uni(U->I, U->stack.allocated, size * sizeof(struct value));

    callstack_recover(U);

    U->stack.size = size;
}



void stackI_set_grow(morphine_coroutine_t U, size_t grow) {
    if (grow == 0) {
        throwI_error(U->I, "Stack grow size is zero");
    }

    U->stack.settings.grow = grow;
}

void stackI_set_limit(morphine_coroutine_t U, size_t limit) {
    if (limit == 0) {
        throwI_error(U->I, "Stack limit is zero");
    }

    U->stack.settings.limit = limit;
}

size_t stackI_space(morphine_coroutine_t U) {
    return stackI_callinfo_space(U, callstackI_info(U));
}

void stackI_push(morphine_coroutine_t U, struct value value) {
    gcI_safe(U->I, value);
    stackI_raise(U, 1);
    if (callstackI_info(U) != NULL) {
        callstackI_info(U)->s.top.p++;
    }

    struct value *stack_value = U->stack.allocated + U->stack.top - 1;
    (*stack_value) = value;
    gcI_reset_safe(U->I);
}

struct value stackI_peek(morphine_coroutine_t U, size_t offset) {
    return stackI_callinfo_peek(U, callstackI_info(U), offset);
}

struct value *stackI_vector(morphine_coroutine_t U, size_t offset, size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct value *p;
    size_t space_size;

    struct callinfo *callinfo = callstackI_info(U);
    if (callinfo == NULL) {
        space_size = U->stack.top;
        p = U->stack.allocated + U->stack.top;
    } else {
        space_size = stackI_callinfo_space(U, callinfo);
        p = callinfo->s.top.p;
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
        stackI_reduce(U, count);
    } else {
        struct callinfo *callinfo = callstackI_info(U);
        size_t size = stackI_callinfo_space(U, callinfo);

        if (count > size) {
            throwI_error(U->I, "Cannot pop from space");
        }

        callinfo->s.top = stackI_ptr(stackI_reduce(U, count));
    }
}

struct value stackI_callinfo_peek(morphine_coroutine_t U, struct callinfo *callinfo, size_t offset) {
    struct value *p;
    size_t space_size;

    if (callinfo == NULL) {
        space_size = U->stack.top;
        p = U->stack.allocated + U->stack.top;
    } else {
        space_size = stackI_callinfo_space(U, callinfo);
        p = callinfo->s.top.p;
    }

    if (offset >= space_size) {
        throwI_error(U->I, "Cannot peek value from space");
    }

    return *(p - offset - 1);
}

size_t stackI_callinfo_space(morphine_coroutine_t U, struct callinfo *callinfo) {
    if (callinfo == NULL) {
        return U->stack.top;
    }

    return (size_t) (callinfo->s.top.p - callinfo->s.space.p);
}
