//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/stack.h"
#include "morphine/core/throw.h"
#include "morphine/core/alloc.h"
#include "morphine/object/state.h"
#include "morphine/object/closure.h"
#include "morphine/object/proto.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"

#define stack_ptr(ptr) ((stackI_ptr) { .p = (ptr) })
#define stack_ptr_save(a, ptr) (ptr) = (stackI_ptr) { .diff = (size_t) ((ptr).p - (a)) }
#define stack_ptr_recover(a, ptr) (ptr) = (stackI_ptr) { .p = (a) + (ptr).diff }

static inline size_t get_grow_size(morphine_state_t S) {
    size_t size = S->settings.stack_grow;

    if (size == 0) {
        throwI_message_panic(S->I, S, "Grow size is zero");
    }

    return size;
}

struct stack stackI_initial(morphine_instance_t I, size_t grow) {
    if (grow == 0) {
        throwI_message_panic(I, NULL, "Grow size is zero");
    }

    struct value *allocated = allocI_uni(
        I,
        NULL,
        0,
        grow * sizeof(struct value)
    );

    return (struct stack) {
        .allocated = allocated,
        .size = grow,
        .top = 0,
        .callstack = NULL
    };
}

static inline void stack_save(struct stack *stack) {
    struct callinfo *current = stack->callstack;
    while (current != NULL) {
        stack_ptr_save(stack->allocated, current->s.callable);
        stack_ptr_save(stack->allocated, current->s.source);
        stack_ptr_save(stack->allocated, current->s.env);
        stack_ptr_save(stack->allocated, current->s.self);
        stack_ptr_save(stack->allocated, current->s.result);
        stack_ptr_save(stack->allocated, current->s.thrown);
        stack_ptr_save(stack->allocated, current->s.args);
        stack_ptr_save(stack->allocated, current->s.values);
        stack_ptr_save(stack->allocated, current->s.space);
        stack_ptr_save(stack->allocated, current->s.top);

        current = current->prev;
    }
}

static inline void stack_recover(struct stack *stack) {
    struct callinfo *current = stack->callstack;
    while (current != NULL) {
        stack_ptr_recover(stack->allocated, current->s.callable);
        stack_ptr_recover(stack->allocated, current->s.source);
        stack_ptr_recover(stack->allocated, current->s.env);
        stack_ptr_recover(stack->allocated, current->s.self);
        stack_ptr_recover(stack->allocated, current->s.result);
        stack_ptr_recover(stack->allocated, current->s.thrown);
        stack_ptr_recover(stack->allocated, current->s.args);
        stack_ptr_recover(stack->allocated, current->s.values);
        stack_ptr_recover(stack->allocated, current->s.space);
        stack_ptr_recover(stack->allocated, current->s.top);

        current = current->prev;
    }
}

static inline stackI_ptr raise(morphine_state_t S, struct value first, size_t size) {
    struct stack *stack = &S->stack;

    if (size == 0) {
        throwI_message_panic(S->I, S, "Raise size is zero");
    }

    stackI_ptr result = stack_ptr(stack->allocated + stack->top);

    *result.p = first;

    stack->top += size;

    if (stack->top >= stack->size) {
        size_t grow = get_grow_size(S);

        size_t osize = stack->size;
        size_t nsize = osize + (((size / grow) + 1) * grow);

        if (nsize >= S->settings.stack_limit) {
            throwI_message_error(S, "Stack overflow");
        }

        stack_save(stack);
        stack_ptr_save(stack->allocated, result);

        stack->allocated = allocI_uni(
            S->I,
            stack->allocated,
            osize * sizeof(struct value),
            nsize * sizeof(struct value)
        );

        stack_ptr_recover(stack->allocated, result);
        stack_recover(stack);

        stack->size = nsize;
    }

    for (size_t i = stack->top - size + 1; i < stack->top; i++) {
        stack->allocated[i] = valueI_nil;
    }

    return result;
}

static inline stackI_ptr reduce(morphine_state_t S, size_t size) {
    if (size > S->stack.top) {
        throwI_message_error(S, "Cannot reduce stack");
    }

    S->stack.top -= size;

    return stack_ptr(S->stack.allocated + S->stack.top);
}

void stackI_shrink(morphine_state_t S) {
    size_t grow = get_grow_size(S);

    if (S->stack.top + grow >= S->stack.size) {
        return;
    }

    size_t size = S->stack.top + grow;

    stack_save(&S->stack);

    S->stack.allocated = allocI_uni(
        S->I,
        S->stack.allocated,
        S->stack.size * sizeof(struct value),
        size * sizeof(struct value)
    );

    stack_recover(&S->stack);

    S->stack.size = size;
}

struct value stackI_extract_callable(morphine_instance_t I, struct value callable) {
    size_t counter = 0;
repeat:;
    if (counter > 1000000) {
        throwI_message_panic(I, NULL, "Possible recursion when extracting callable");
    }

    struct value metacall = valueI_nil;
    if (metatableI_test(I, callable, MF_CALL, &metacall)) {
        callable = metacall;
        counter++;
        goto repeat;
    }

    struct closure *closure = valueI_safe_as_closure(callable, NULL);
    if (closure != NULL) {
        callable = closure->callable;
        counter++;
        goto repeat;
    }

    if (valueI_is_native(callable) || valueI_is_proto(callable)) {
        return callable;
    }

    return valueI_nil;
}

void stackI_call(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t argc,
    struct value *args,
    size_t pop_size
) {
    // get source and calc values size

    struct value source = stackI_extract_callable(S->I, callable);

    if (valueI_is_nil(source)) {
        throwI_errorf(S, "Unable to call %s", valueI_type2string(S->I, callable.type));
    }

    size_t values_size = 6 + argc;

    if (valueI_is_proto(source)) {
        struct proto *proto = valueI_as_proto(source);

        if (argc != proto->arguments_count) {
            throwI_message_error(S, "Wrong arguments count");
        }

        values_size += proto->slots_count + proto->params_count;
    }

    // get env

    struct value env = valueI_object_or_panic(S->I, S, S->I->env);
    if (stackI_callinfo(S) != NULL) {
        env = *stackI_callinfo(S)->s.env.p;
    }

    // create callinfo

    if (pop_size > stackI_space_size(S)) {
        throwI_message_error(S, "Cannot pop values after call");
    }

    stackI_ptr base = raise(S, callable, values_size);

    struct callinfo temp = {
        .s.base = base,
        .s.source = stack_ptr(base.p + 1),
        .s.env = stack_ptr(base.p + 2),
        .s.self = stack_ptr(base.p + 3),
        .s.result = stack_ptr(base.p + 4),
        .s.thrown = stack_ptr(base.p + 5),
        .s.args = stack_ptr(base.p + 6),
        .s.values = stack_ptr(base.p + 6 + argc),
        .s.space = stack_ptr(S->stack.allocated + S->stack.top),
        .s.top = stack_ptr(S->stack.allocated + S->stack.top),
        .pop_size = pop_size,
        .arguments_count = argc,
        .pc.position = 0,
        .pc.callstate = 0,
        .catch.enable = false,
        .catch.callstate = 0,
        .exit = false,
        .prev = stackI_callinfo(S)
    };

    struct callinfo *callinfo = gcI_get_hot_callinfo(S->I);
    if (callinfo == NULL) {
        callinfo = allocI_uni(S->I, NULL, 0, sizeof(struct callinfo));
    }

    (*callinfo) = temp;

    stackI_callinfo(S) = callinfo;

    // init callinfo stack region

    *callinfo->s.source.p = source;
    *callinfo->s.env.p = env;
    *callinfo->s.self.p = self;

    for (size_t i = 0; i < argc; i++) {
        callinfo->s.args.p[i] = args[i];
    }
}

void stackI_call_pop(morphine_state_t S) {
    struct callinfo *callinfo = stackI_callinfo_or_error(S);
    size_t pop_size = callinfo->pop_size;

    reduce(S, (size_t) (callinfo->s.top.p - callinfo->s.base.p));

    stackI_callinfo(S) = callinfo->prev;

    gcI_dispose_callinfo(S->I, callinfo);

    stackI_pop(S, pop_size);

    if (stackI_callinfo(S) == NULL) {
        stateI_kill(S);
    }
}

void stackI_callinfo_free(morphine_instance_t I, struct callinfo *callinfo) {
    allocI_uni(I, callinfo, sizeof(struct callinfo), 0);
}

size_t stackI_space_size(morphine_state_t S) {
    if (stackI_callinfo(S) == NULL) {
        return S->stack.top;
    } else {
        return (size_t) (stackI_callinfo(S)->s.top.p - stackI_callinfo(S)->s.space.p);
    }
}

void stackI_push(morphine_state_t S, struct value value) {
    if (stackI_callinfo(S) == NULL) {
        raise(S, value, 1);
    } else {
        raise(S, value, 1);
        stackI_callinfo(S)->s.top.p++;
    }
}

struct value stackI_peek(morphine_state_t S, size_t offset) {
    struct value *p;
    size_t space_size;

    if (stackI_callinfo(S) == NULL) {
        space_size = S->stack.top;
        p = S->stack.allocated + S->stack.top;
    } else {
        struct callinfo *callinfo = stackI_callinfo(S);
        space_size = stackI_space_size(S);
        p = callinfo->s.top.p;
    }

    if (offset >= space_size) {
        throwI_message_error(S, "Cannot peek value from space");
    }

    return *(p - offset - 1);
}

struct value *stackI_vector(morphine_state_t S, size_t offset, size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct value *p;
    size_t space_size;

    if (stackI_callinfo(S) == NULL) {
        space_size = S->stack.top;
        p = S->stack.allocated + S->stack.top;
    } else {
        struct callinfo *callinfo = stackI_callinfo(S);
        space_size = stackI_space_size(S);
        p = callinfo->s.top.p;
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

    if (stackI_callinfo(S) == NULL) {
        reduce(S, count);
    } else {
        struct callinfo *callinfo = stackI_callinfo(S);
        size_t size = stackI_space_size(S);

        if (count > size) {
            throwI_message_error(S, "Cannot pop from space");
        }

        callinfo->s.top = reduce(S, count);
    }
}
