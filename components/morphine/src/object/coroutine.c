//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/coroutine.h"
#include "morphine/core/instance.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/misc/localstorage.h"
#include "morphine/object/closure.h"
#include "morphine/object/function.h"
#include "morphine/object/vector.h"
#include "morphine/utils/assert.h"

#define stack_overflow(I) do { throwI_error((I), "stack overflow"); } while (0)

// coroutine

static void attach(morphine_coroutine_t U) {
    if (U == NULL) {
        return;
    }

    morphine_instance_t I = U->I;

    morphine_coroutine_t current = I->interpreter.coroutines;
    while (current != NULL && current != U) {
        current = current->prev;
    }

    if (current != NULL) {
        return;
    }

    U->prev = I->interpreter.coroutines;
    I->interpreter.coroutines = U;
}

static void detach(morphine_coroutine_t U) {
    if (U == NULL) {
        return;
    }

    morphine_instance_t I = U->I;

    morphine_coroutine_t last = NULL;
    morphine_coroutine_t current = I->interpreter.coroutines;
    while (current != NULL && current != U) {
        last = current;
        current = current->prev;
    }

    if (current == NULL) {
        return;
    }

    if (last == NULL) {
        I->interpreter.coroutines = current->prev;
    } else {
        last->prev = current->prev;
    }

    if (current == I->interpreter.next) {
        I->interpreter.next = current->prev;
    }

    current->prev = NULL;
}

static void coroutine_attach(morphine_coroutine_t U) {
    if (U->status == COROUTINE_STATUS_DETACHED) {
        attach(U);
        U->status = COROUTINE_STATUS_RUNNING;
    }
}

// stack

struct stack_value {
    bool from_stack;
    union {
        struct value *value;
        ml_size offset;
    };
};

static inline struct stack_value save_stack_value(morphine_coroutine_t U, struct value *value) {
    bool is_stack_value = value >= U->stack.allocated && value < U->stack.allocated + U->stack.top;

    if (value == NULL || !is_stack_value) {
        return (struct stack_value) {
            .from_stack = false,
            .value = value,
        };
    }

    return (struct stack_value) {
        .from_stack = true,
        .offset = (ml_size) (value - U->stack.allocated),
    };
}

static inline struct value *extract_stack_value(morphine_coroutine_t U, struct stack_value stack_value) {
    if (stack_value.from_stack) {
        return U->stack.allocated + stack_value.offset;
    }

    return stack_value.value;
}

static inline void callstack_recover(morphine_coroutine_t U, struct value *old_stack, ml_size old_top) {
    struct callframe *current = U->callstack.frame;
    while (current != NULL) {
#define stackI_stack_ptr_recover(ptr) do { (ptr) = U->stack.allocated + ((size_t) ((ptr) - old_stack)); } while(0)
#define stackI_ptr_recover(ptr) do { if(ptr >= old_stack && ptr < (old_stack + old_top)) { stackI_stack_ptr_recover(ptr); } } while(0)

        stackI_stack_ptr_recover(current->s.stack.base);
        stackI_stack_ptr_recover(current->s.stack.top);
        stackI_ptr_recover(current->s.direct.callable);
        stackI_ptr_recover(current->s.direct.args);

#undef stackI_stack_ptr_recover
#undef stackI_ptr_recover

        current = current->prev;
    }
}

static struct value *stack_raise(morphine_coroutine_t U, ml_size size) {
    ml_size new_top = mm_overflow_opc_add(U->stack.top, size, stack_overflow(U->I));
    if (new_top > U->stack.size) {
        ml_size diff = new_top - U->stack.size;
        ml_size factor = (diff / MPARAM_STACK_GROW) + (diff % MPARAM_STACK_GROW == 0 ? 0 : 1);

        ml_size new_size = mm_overflow_opc_mul(factor, MPARAM_STACK_GROW, stack_overflow(U->I));
        new_size = mm_overflow_opc_add(new_size, U->stack.size, stack_overflow(U->I));
        if (new_size >= U->stack.settings.limit) {
            stack_overflow(U->I);
        }

        struct value *old_allocated = U->stack.allocated;
        ml_size old_top = U->stack.top;

        U->stack.settings.allow_reduce_stack = false;
        U->I->throw.fix.stack = U;

        U->stack.allocated = allocI_vec(U->I, U->stack.allocated, new_size, sizeof(struct value));

        U->stack.settings.allow_reduce_stack = true;
        U->I->throw.fix.stack = NULL;

        if (U->stack.allocated != old_allocated) {
            callstack_recover(U, old_allocated, old_top);
        }

        U->stack.size = new_size;
    }

    U->stack.top = new_top;

    for (ml_size i = U->stack.top - size; i < U->stack.top; i++) {
        U->stack.allocated[i] = valueI_nil;
    }

    return U->stack.allocated + (U->stack.top - size);
}

static struct value *stack_reduce(morphine_coroutine_t U, ml_size size) {
    if (size > U->stack.top) {
        throwI_error(U->I, "cannot reduce stack");
    }

    U->stack.top -= size;
    return U->stack.allocated + U->stack.top;
}

static inline ml_size stack_space(morphine_coroutine_t U) {
    struct callframe *frame = U->callstack.frame;
    if (frame == NULL) {
        return U->stack.top;
    }

    return (ml_size) (frame->s.stack.top - frame->s.stack.base);
}

static inline struct value *stack_push(morphine_coroutine_t U, ml_size count) {
    struct value *top = stack_raise(U, count);
    if (U->callstack.frame != NULL) {
        U->callstack.frame->s.stack.top += count;
    }

    return top;
}

static inline void stack_pop(morphine_coroutine_t U, ml_size count) {
    if (count == 0) {
        return;
    }

    struct value *top = stack_reduce(U, count);
    if (U->callstack.frame != NULL) {
        U->callstack.frame->s.stack.top = top;
    }
}

static inline struct value *stack_vector(morphine_coroutine_t U, ml_size offset, ml_size size) {
    if (size == 0) {
        return NULL;
    }

    ml_size space_size = stack_space(U);

    struct value *p;
    if (U->callstack.frame != NULL) {
        p = U->callstack.frame->s.stack.top;
    } else {
        p = U->stack.allocated + U->stack.top;
    }

    if (offset + size > space_size) {
        throwI_error(U->I, size > 1 ? "cannot peek values from stack" : "cannot peek value from stack");
    }

    return p - offset - size;
}

// callstack

static void callframe_dispose(morphine_coroutine_t U, struct callframe *frame) {
    mm_overflow_add(U->callstack.cache.size, 1) {
        allocI_free(U->I, frame);
        return;
    }

    frame->prev = U->callstack.cache.pool;
    U->callstack.cache.pool = frame;
    U->callstack.cache.size++;
}

static struct callframe *callframe_get(morphine_coroutine_t U) {
    struct callframe *frame = U->callstack.cache.pool;

    if (frame != NULL) {
        U->callstack.cache.pool = frame->prev;
        frame->prev = NULL;

        mm_assert(U->I, U->callstack.cache.size > 0, "zero callframe cache");
        U->callstack.cache.size--;
    } else {
        frame = allocI_uni(U->I, NULL, sizeof(struct callframe));
    }

    return frame;
}

static inline struct value extract_callable(
    morphine_instance_t I,
    struct value callable,
    const char *error,
    bool error_before
) {
    struct closure *closure = valueI_safe_as_closure(callable, NULL);
    if (closure != NULL) {
        callable = closure->callable;
    }

    if (valueI_is_native(callable) || valueI_is_function(callable)) {
        return callable;
    }

    const char *type = valueI_type(I, callable, false);
    if (error_before) {
        throwI_errorf(I, "%s %s", error, type);
    } else {
        throwI_errorf(I, "%s %s", type, error);
    }
}

static struct callframe *callstack_frame_or_error(morphine_coroutine_t U) {
    if (U->callstack.frame == NULL) {
        throwI_error(U->I, "require call frame");
    }

    return U->callstack.frame;
}

// other stuff

morphine_coroutine_t coroutineI_create(morphine_instance_t I, struct string *name, struct value env) {
    if (name == NULL) {
        throwI_error(I, "coroutine name is null");
    }

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(name));
    gcI_safe(I, env);

    // create
    morphine_coroutine_t result = allocI_uni(I, NULL, sizeof(struct coroutine));
    (*result) = (struct coroutine) {
        .name = name,
        .status = COROUTINE_STATUS_DETACHED,
        .stack.allocated = NULL,
        .stack.size = 0,
        .stack.top = 0,
        .stack.settings.limit = I->settings.coroutines.stack.limit,
        .stack.settings.allow_reduce_stack = true,
        .callstack.access = 0,
        .callstack.size = 0,
        .callstack.frame = NULL,
        .callstack.uninited = NULL,
        .callstack.result = valueI_nil,
        .callstack.cache.size = 0,
        .callstack.cache.pool = NULL,
        .env = env,
        .exception = NULL,
        .prev = NULL,
        .I = I,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_COROUTINE);

    gcI_safe_exit(I);

    return result;
}

void coroutineI_free(morphine_instance_t I, morphine_coroutine_t coroutine) {
    {
        struct callframe *current = coroutine->callstack.frame;
        while (current != NULL) {
            struct callframe *prev = current->prev;
            allocI_free(I, current);
            current = prev;
        }
    }

    {
        struct callframe *current = coroutine->callstack.cache.pool;
        while (current != NULL) {
            struct callframe *prev = current->prev;
            allocI_free(I, current);
            current = prev;
        }
    }

    allocI_free(I, coroutine->callstack.uninited);
    allocI_free(I, coroutine->stack.allocated);
    allocI_free(I, coroutine);
}

void coroutineI_fix_stack(morphine_coroutine_t U) {
    U->stack.settings.allow_reduce_stack = true;
}

void coroutineI_fix_callstack(morphine_coroutine_t U) {
    if (U->callstack.uninited == NULL) {
        return;
    }

    struct callframe *frame = U->callstack.uninited;
    bool pop_callable = frame->uninited.pop_callable;
    bool pop_args = frame->uninited.pop_args;
    ml_size args_size = frame->uninited.arguments_count;

    callframe_dispose(U, frame);
    U->callstack.uninited = NULL;

    if (pop_args) {
        stack_pop(U, args_size);
    }

    if (pop_callable) {
        stack_pop(U, 1);
    }
}

void coroutineI_detach(morphine_coroutine_t U) {
    callstackI_drop(U, NULL);
    detach(U);
    U->status = COROUTINE_STATUS_DETACHED;
    callstackI_update_access(U);
}

void coroutineI_suspend(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_DETACHED:
        case COROUTINE_STATUS_KILLING:
        case COROUTINE_STATUS_SUSPENDED: return;
        case COROUTINE_STATUS_RUNNING: U->status = COROUTINE_STATUS_SUSPENDED; return;
    }
}

void coroutineI_resume(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_DETACHED:
        case COROUTINE_STATUS_KILLING:
        case COROUTINE_STATUS_RUNNING: return;
        case COROUTINE_STATUS_SUSPENDED: U->status = COROUTINE_STATUS_RUNNING; return;
    }
}

void coroutineI_kill(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED: U->status = COROUTINE_STATUS_KILLING; return;
        case COROUTINE_STATUS_DETACHED:
        case COROUTINE_STATUS_KILLING: return;
    }
}

bool coroutineI_isalive(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_KILLING:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED: return true;
        case COROUTINE_STATUS_DETACHED: return false;
    }

    throwI_panic(U->I, "unknown coroutine status");
}

struct exception *coroutineI_exception(morphine_coroutine_t U) {
    struct exception *result = U->exception;
    U->exception = NULL;
    return result;
}

const char *coroutineI_status2string(morphine_coroutine_t U, coroutine_status_t status) {
    switch (status) {
        case COROUTINE_STATUS_DETACHED: return "detached";
        case COROUTINE_STATUS_RUNNING: return "running";
        case COROUTINE_STATUS_SUSPENDED: return "suspended";
        case COROUTINE_STATUS_KILLING: return "killing";
    }

    throwI_panic(U->I, "unknown coroutine status");
}

void stackI_set_limit(morphine_coroutine_t U, ml_size limit) {
    U->stack.settings.limit = limit;
}

void stackI_reduce_stack(morphine_coroutine_t U, bool emergency) {
    if (!U->stack.settings.allow_reduce_stack) {
        return;
    }

    ml_size size;
    if (emergency) {
        size = U->stack.top;
    } else {
        size = mm_overflow_opc_add(U->stack.top, MPARAM_STACK_GROW, return);
        if (size >= U->stack.size) {
            return;
        }
    }

    U->stack.settings.allow_reduce_stack = false;
    U->I->throw.fix.stack = U;

    struct value *saved_allocated = U->stack.allocated;
    ml_size saved_top = U->stack.top;

    U->stack.allocated = allocI_vec(U->I, U->stack.allocated, size, sizeof(struct value));

    if (U->stack.allocated != saved_allocated) {
        callstack_recover(U, saved_allocated, saved_top);
    }

    U->stack.size = size;
    U->stack.settings.allow_reduce_stack = true;
    U->I->throw.fix.stack = NULL;
}

void stackI_reduce_cache(morphine_coroutine_t U, bool emergency) {
    while (U->callstack.cache.pool != NULL) {
        if (!emergency && U->callstack.cache.size < MPARAM_CALL_CACHE_LIMIT) {
            break;
        }

        struct callframe *frame = callframe_get(U);
        allocI_free(U->I, frame);
    }
}

ml_size stackI_space(morphine_coroutine_t U) {
    callstackI_check_access(U);
    return stack_space(U);
}

void stackI_push(morphine_coroutine_t U, struct value value) {
    callstackI_check_access(U);

    gcI_safe_enter(U->I);
    gcI_safe(U->I, value);

    struct value *top = stack_push(U, 1);
    (*top) = value;

    gcI_safe_exit(U->I);
}

void stackI_pop(morphine_coroutine_t U, ml_size count) {
    callstackI_check_access(U);
    stack_pop(U, count);
}

struct value stackI_peek(morphine_coroutine_t U, ml_size offset) {
    callstackI_check_access(U);
    return *stack_vector(U, offset, 1);
}

void stackI_replace(morphine_coroutine_t U, ml_size offset, struct value value) {
    callstackI_check_access(U);
    *stack_vector(U, offset, 1) = value;
}

void stackI_rotate(morphine_coroutine_t U, ml_size count) {
    callstackI_check_access(U);

    if (count == 0) {
        return;
    }

    if (count > stack_space(U)) {
        throwI_error(U->I, "cannot rotate stack");
    }

    struct value *values = stack_vector(U, 0, count);
    struct value temp = values[count - 1];
    for (ml_size i = 0; i < count - 1; i++) {
        values[count - i - 1] = values[count - i - 2];
    }
    values[0] = temp;
}

void callstackI_check_access(morphine_coroutine_t U) {
    if (U->status == COROUTINE_STATUS_KILLING) {
        throwI_error(U->I, "attempt to access while killing coroutine");
    }

    if (U->status != COROUTINE_STATUS_DETACHED && U->I->interpreter.context != U) {
        throwI_error(U->I, "attempt to access outside current coroutine");
    }

    if (U->callstack.size != U->callstack.access) {
        throwI_error(U->I, "attempt to access outside current call frame");
    }
}

void callstackI_update_access(morphine_coroutine_t U) {
    U->callstack.access = U->callstack.size;
}

void callstackI_call(morphine_coroutine_t U, struct value *callable, struct value *args, ml_size argc, ml_size pop_size) {
    callstackI_check_access(U);

    morphine_instance_t I = U->I;
    if (argc > MPARAM_CALLABLE_ARGS_LIMIT) {
        throwI_error(I, "too many arguments");
    }

    if (pop_size > stack_space(U)) {
        throwI_error(I, "unable to pop after call");
    }

    struct stack_value stack_callable = save_stack_value(U, callable);
    struct stack_value stack_args = save_stack_value(U, args);

    // precall

    gcI_safe_enter(I);

    struct value mt_field;
    struct value mt_args[2];
    if (mm_unlikely(metatableI_builtin_test(I, *callable, MORPHINE_METAFIELD_CALL, &mt_field))) { // check metafield
        struct vector *vector = gcI_safe_obj(I, vector, vectorI_create(I, argc));
        for (ml_size i = 0; i < argc; i++) {
            vectorI_set(I, vector, i, extract_stack_value(U, stack_args)[i]);
        }

        mt_args[0] = *extract_stack_value(U, stack_callable);
        mt_args[1] = valueI_object(vector);

        stack_callable = save_stack_value(U, &mt_field);
        stack_args = save_stack_value(U, mt_args);
        argc = 2;
    }

    ml_size raise = 0;

    { // extract source
        struct value source = extract_callable(U->I, *extract_stack_value(U, stack_callable), "unable to call", true);
        if (valueI_is_function(source)) {
            struct function *function = valueI_as_function(source);

            if (!function->complete) {
                throwI_error(U->I, "incomplete function");
            }

            raise = function->stack_size;
        }
    }

    bool pop_callable = false;
    bool pop_args = false;

    { // prepare stack
        ml_size push_size_callable = stack_callable.from_stack ? 0 : 1;
        ml_size push_size_args = stack_args.from_stack ? 0 : argc;
        ml_size push_size = mm_overflow_opc_add(push_size_callable, push_size_args, stack_overflow(U->I));

        struct value *values = stack_push(U, push_size);

        if (!stack_callable.from_stack) {
            values[0] = *extract_stack_value(U, stack_callable);
            stack_callable = save_stack_value(U, stack_vector(U, push_size - 1, 1));
            pop_callable = true;
        }

        if (!stack_args.from_stack && argc > 0) {
            for (ml_size i = 0; i < argc; i++) {
                values[i + push_size_callable] = extract_stack_value(U, stack_args)[i];
            }

            stack_args = save_stack_value(U, stack_vector(U, 0, argc));
            pop_args = true;
        }
    }

    gcI_safe_exit(I);

    // create frame

    mm_overflow_add(U->callstack.size, 1) {
        throwI_error(I, "callstack overflow");
    }

    struct callframe *frame = callframe_get(U);
    (*frame) = (struct callframe) {
        .uninited.pop_callable = pop_callable,
        .uninited.pop_args = pop_args,
        .uninited.arguments_count = argc,
    };

    U->callstack.uninited = frame;
    U->I->throw.fix.callstack = U;

    struct value *base = stack_raise(U, raise);

    U->callstack.uninited = NULL;
    U->I->throw.fix.callstack = NULL;

    (*frame) = (struct callframe) {
        .s.stack.base = base,
        .s.stack.top = U->stack.allocated + U->stack.top,
        .s.direct.callable = extract_stack_value(U, stack_callable),
        .s.direct.args = extract_stack_value(U, stack_args),
        .params.pop_callable = pop_callable,
        .params.pop_args = pop_args,
        .params.catch_enabled = false,
        .params.catch_crash = false,
        .params.catch_state = 0,
        .params.arguments_count = argc,
        .params.pop_size = pop_size,
        .pc.position = 0,
        .pc.state = 0,
        .prev = U->callstack.frame,
    };

    U->callstack.frame = frame;
    U->callstack.size++;

    if (frame->prev == NULL) {
        coroutine_attach(U);
    }
}

void callstackI_call_api(morphine_coroutine_t U, ml_size argc) {
    callstackI_check_access(U);

    struct value *callable = stack_vector(U, argc, 1);
    struct value *args = argc > 0 ? stack_vector(U, 0, argc) : NULL;

    callstackI_call(U, callable, args, argc, argc + 1);
}

void callstackI_pop(morphine_coroutine_t U, struct value result) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);
    U->callstack.result = result;
    callstackI_drop(U, frame->prev);

    if (U->callstack.frame == NULL) {
        coroutineI_kill(U);
    }
}

void callstackI_drop(morphine_coroutine_t U, struct callframe *frame) {
    while (U->callstack.frame != NULL && U->callstack.frame != frame) {
        struct callframe *current = U->callstack.frame;
        localstorageI_clear(U->I, current);

        ml_size pop_size = current->params.pop_size;
        pop_size += current->params.pop_callable ? 1 : 0;
        pop_size += current->params.pop_args ? current->params.arguments_count : 0;

        stack_reduce(U, stack_space(U));
        U->callstack.frame = current->prev;
        U->callstack.size--;

        callframe_dispose(U, current);
        stack_pop(U, pop_size);
    }
}

struct value callstackI_extract_callable(morphine_instance_t I, struct value callable) {
    return extract_callable(I, callable, "isn't callable type", false);
}

void callstackI_continue(morphine_coroutine_t U, ml_size state) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);
    frame->pc.state = state;
}

ml_size callstackI_state(morphine_coroutine_t U) {
    callstackI_check_access(U);
    return callstack_frame_or_error(U)->pc.state;
}

ml_size callstackI_args(morphine_coroutine_t U) {
    callstackI_check_access(U);
    return callstack_frame_or_error(U)->params.arguments_count;
}

struct value callstackI_get_arg(morphine_coroutine_t U, ml_size index) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);
    if (index >= frame->params.arguments_count) {
        throwI_error(U->I, "argument index out of bounce");
    }

    return frame->s.direct.args[index];
}

struct value callstackI_callable(morphine_coroutine_t U) {
    callstackI_check_access(U);
    return *callstack_frame_or_error(U)->s.direct.callable;
}

struct value callstackI_result(morphine_coroutine_t U) {
    callstackI_check_access(U);
    struct value result = U->callstack.result;
    U->callstack.result = valueI_nil;
    return result;
}

void callstackI_set_result(morphine_coroutine_t U, struct value value) {
    callstackI_check_access(U);
    U->callstack.result = value;
}

void callstackI_catchable(morphine_coroutine_t U, ml_size callstate) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);

    frame->params.catch_enabled = true;
    frame->params.catch_crash = false;
    frame->params.catch_state = callstate;
}

void callstackI_crashable(morphine_coroutine_t U) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);

    frame->params.catch_enabled = true;
    frame->params.catch_crash = true;
    frame->params.catch_state = 0;
}

void callstackI_uncatch(morphine_coroutine_t U) {
    callstackI_check_access(U);
    struct callframe *frame = callstack_frame_or_error(U);

    frame->params.catch_enabled = false;
    frame->params.catch_crash = false;
    frame->params.catch_state = 0;
}
