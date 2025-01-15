//
// Created by why-iskra on 04.04.2024.
//

#include "morphine/core/callstack.h"
#include "morphine/core/instance.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/closure.h"
#include "morphine/object/vector.h"
#include "morphine/gc/cache.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/misc/localstorage.h"
#include "morphine/utils/overflow.h"

struct stack_value {
    bool from_stack;
    union {
        struct value *value;
        size_t offset;
    };
};

static inline struct stack_value save_value(morphine_coroutine_t U, struct value *value) {
    bool is_stack_value =
        value >= U->stack.array.allocated && value < U->stack.array.allocated + U->stack.array.top;

    if (value == NULL || !is_stack_value) {
        return (struct stack_value) {
            .from_stack = false,
            .value = value
        };
    }

    return (struct stack_value) {
        .from_stack = true,
        .offset = (size_t) (value - U->stack.array.allocated)
    };
}

static inline struct value *extract_value(morphine_coroutine_t U, struct stack_value stack_value) {
    if (stack_value.from_stack) {
        return U->stack.array.allocated + stack_value.offset;
    }

    return stack_value.value;
}

static inline struct stack_value peek_stack_value(morphine_coroutine_t U, size_t offset) {
    return save_value(U, stackI_unsafe_peek(U, offset));
}

static inline bool stack_value_is_null(struct stack_value value) {
    return !value.from_stack && value.value == NULL;
}

// callable

static inline struct value extract_callable(
    morphine_instance_t I,
    struct value callable,
    const char *error,
    bool error_before
) {
    size_t counter = 0;
repeat:;
    if (counter > MPARAM_FAKE_RECURSION_DEEP) {
        throwI_error(I, "possible recursion while extracting callable");
    }

    struct closure *closure = valueI_safe_as_closure(callable, NULL);
    if (closure != NULL) {
        callable = closure->callable;
        counter++;
        goto repeat;
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

static inline void internal_call(
    morphine_coroutine_t U,
    ml_size args_count,
    size_t pop_size,
    struct stack_value callable,
    struct stack_value env,
    struct stack_value self,
    struct stack_value args
) {
    morphine_instance_t I = U->I;

    if (args_count > MLIMIT_CALLABLE_ARGS) {
        throwI_error(I, "too many args");
    }

    // get source and calc values size

    struct value source = extract_callable(I, *extract_value(U, callable), "unable to call", true);

    size_t slots_count = 0;
    size_t params_count = 0;

    if (valueI_is_function(source)) {
        struct function *function = valueI_as_function(source);

        if (!function->complete) {
            throwI_error(I, "incomplete function");
        }

        slots_count = function->slots_count;
        params_count = function->params_count;
    }

    // get env

    struct stack_value extracted_env;
    if (stack_value_is_null(env)) {
        if (callstackI_info(U) != NULL) {
            extracted_env = save_value(U, callstackI_info(U)->s.direct.env);
        } else {
            extracted_env = save_value(U, &U->env);
        }
    } else {
        extracted_env = env;
    }

    // create callinfo

    struct callinfo *callinfo = gcI_cache_callinfo(I);
    if (callinfo == NULL) {
        callinfo = allocI_uni(I, NULL, sizeof(struct callinfo));
    }

    U->callstack.uninit_callinfo = callinfo;

    size_t raise_size = 1;
    raise_size = overflow_op_add(raise_size, slots_count, SIZE_MAX, throwI_error(I, "stack raise overflow"));
    raise_size = overflow_op_add(raise_size, params_count, SIZE_MAX, throwI_error(I, "stack raise overflow"));

    struct value *base = stackI_raise(U, raise_size);

    (*callinfo) = (struct callinfo) {
        .s.stack.base = base,
        .s.stack.space = (base + 1),
        .s.stack.top = (U->stack.array.allocated + U->stack.array.top),

        .s.direct.callable = extract_value(U, callable),
        .s.direct.env = extract_value(U, extracted_env),
        .s.direct.self = extract_value(U, self),
        .s.direct.args = extract_value(U, args),

        .info.pop_size = pop_size,
        .info.arguments_count = args_count,
        .pc.position = 0,
        .pc.state = 0,
        .catch.enable = false,
        .catch.crash = false,
        .catch.state = 0,
        .exit = true,
        .prev = callstackI_info(U)
    };

    // init callinfo stack region

    *callinfo->s.stack.source = source;

    // add callinfo

    callstackI_info(U) = callinfo;
    U->callstack.size++;
    U->callstack.uninit_callinfo = NULL;
}

struct callstack callstackI_prototype(void) {
    return (struct callstack) {
        .callinfo = NULL,
        .uninit_callinfo = NULL,
        .size = 0,
    };
}

void callstackI_destruct(morphine_instance_t I, struct callstack *callstack) {
    struct callinfo *callinfo = callstack->callinfo;
    while (callinfo != NULL) {
        struct callinfo *prev = callinfo->prev;
        callstackI_callinfo_free(I, callinfo);
        callinfo = prev;
    }

    callstackI_callinfo_free(I, callstack->uninit_callinfo);
    callstack->callinfo = NULL;
    callstack->size = 0;
}

void callstackI_callinfo_free(morphine_instance_t I, struct callinfo *callinfo) {
    allocI_free(I, callinfo);
}

void callstackI_pop(morphine_coroutine_t U) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    localstorageI_clear(U);

    size_t pop_size = callinfo->info.pop_size;

    stackI_reduce(U, (size_t) (callinfo->s.stack.top - callinfo->s.stack.base));
    callstackI_info(U) = callinfo->prev;
    U->callstack.size--;

    gcI_cache_dispose_callinfo(U->I, callinfo);

    stackI_pop(U, pop_size);

    if (callstackI_info(U) == NULL) {
        coroutineI_kill(U);
    }
}

void callstackI_throw_fix(morphine_coroutine_t U) {
    if (U->callstack.uninit_callinfo != NULL) {
        gcI_cache_dispose_callinfo(U->I, U->callstack.uninit_callinfo);
        U->callstack.uninit_callinfo = NULL;
    }
}

struct value callstackI_extract_callable(morphine_instance_t I, struct value callable) {
    return extract_callable(I, callable, "isn't callable type", false);
}

struct value callstackI_result(morphine_coroutine_t U) {
    struct value result = U->result;
    U->result = valueI_nil;
    return result;
}

void callstackI_set_result(morphine_coroutine_t U, struct value result) {
    U->result = result;
}

void callstackI_return(morphine_coroutine_t U, struct value value) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    U->result = value;
    callinfo->exit = true;
}

void callstackI_continue(morphine_coroutine_t U, ml_callstate state) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    callinfo->pc.state = state;
    callinfo->exit = false;
}

ml_callstate callstackI_state(morphine_coroutine_t U) {
    return callstackI_info_or_error(U)->pc.state;
}

void callstackI_call_unsafe(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    struct value *args,
    ml_size argc,
    size_t pop_size
) {
    if (argc > MLIMIT_CALLABLE_ARGS) {
        throwI_error(U->I, "too many args");
    }

    struct value mt_field;
    if (mm_unlikely(metatableI_builtin_test(U->I, callable, MORPHINE_METAFIELD_CALL, &mt_field))) {
        gcI_safe_enter(U->I);
        struct vector *vector = gcI_safe_obj(U->I, vector, vectorI_create(U->I, argc));

        for (ml_size i = 0; i < argc; i++) {
            vectorI_set(U->I, vector, i, args[i]);
        }

        stackI_push(U, mt_field);
        stackI_push(U, callable);
        stackI_push(U, self);
        stackI_push(U, valueI_object(vector));

        overflow_add(pop_size, 4, SIZE_MAX) {
            throwI_error(U->I, "pop overflow");
        }

        struct stack_value call_arg_callable = peek_stack_value(U, 3);
        struct stack_value call_arg_env = save_value(U, NULL);
        struct stack_value call_arg_self = peek_stack_value(U, 2);
        struct stack_value call_arg_args = peek_stack_value(U, 1);

        internal_call(
            U, 2, 4 + pop_size,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );

        gcI_safe_exit(U->I);
    } else {
        overflow_add(pop_size, argc + 2, SIZE_MAX) {
            throwI_error(U->I, "pop overflow");
        }

        stackI_push(U, callable);
        stackI_push(U, self);

        for (ml_size i = 0; i < argc; i++) {
            stackI_push(U, args[i]);
        }

        struct stack_value call_arg_callable = peek_stack_value(U, argc + 1);
        struct stack_value call_arg_env = save_value(U, NULL);
        struct stack_value call_arg_self = peek_stack_value(U, argc);
        struct stack_value call_arg_args = argc == 0 ? save_value(U, NULL) : peek_stack_value(U, argc - 1);

        internal_call(
            U, argc, (2 + argc) + pop_size,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );
    }
}

void callstackI_call_from_api(
    morphine_coroutine_t U,
    bool has_env,
    bool has_self,
    ml_size argc
) {
    if (argc > MLIMIT_CALLABLE_ARGS) {
        throwI_error(U->I, "too many args");
    }

    size_t callable_offset;
    size_t env_offset;
    size_t self_offset;

    {
        if (has_env && has_self) {
            callable_offset = argc + 2;
        } else if (has_env || has_self) {
            callable_offset = argc + 1;
        } else {
            callable_offset = argc;
        }

        if (has_env) {
            env_offset = callable_offset - 1;
        } else {
            env_offset = callable_offset;
        }

        if (has_self) {
            self_offset = env_offset - 1;
        } else {
            self_offset = env_offset;
        }
    }

    struct callinfo *callinfo = callstackI_info(U);
    struct value callable = stackI_peek(U, callable_offset);
    struct value mt_field;
    if (mm_unlikely(metatableI_builtin_test(U->I, callable, MORPHINE_METAFIELD_CALL, &mt_field))) {
        gcI_safe_enter(U->I);
        struct vector *vector = gcI_safe_obj(U->I, vector, vectorI_create(U->I, argc));

        for (ml_size i = 0; i < argc; i++) {
            vectorI_set(U->I, vector, i, stackI_peek(U, argc - i - 1));
        }

        stackI_push(U, mt_field);

        if (has_self) {
            stackI_push(U, stackI_peek(U, self_offset));
        } else if (callinfo != NULL) {
            stackI_push(U, *callinfo->s.direct.self);
        } else {
            stackI_push(U, valueI_nil);
        }

        stackI_push(U, valueI_object(vector));

        struct stack_value call_arg_callable = peek_stack_value(U, 2);
        struct stack_value call_arg_env = has_env ? peek_stack_value(U, env_offset + 3) : save_value(U, NULL);
        struct stack_value call_arg_self = peek_stack_value(U, callable_offset + 3);
        struct stack_value call_arg_args = peek_stack_value(U, 1);

        internal_call(
            U, 2, callable_offset + 4,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );

        gcI_safe_exit(U->I);
    } else {
        struct stack_value call_arg_self;
        size_t offset = 0;
        if (has_self) {
            call_arg_self = peek_stack_value(U, self_offset);
        } else if (callinfo != NULL) {
            call_arg_self = save_value(U, callinfo->s.direct.self);
        } else {
            stackI_push(U, valueI_nil);
            call_arg_self = peek_stack_value(U, 0);
            offset = 1;
        }

        struct stack_value call_arg_callable = peek_stack_value(U, callable_offset + offset);
        struct stack_value call_arg_env =
            has_env ? peek_stack_value(U, env_offset + offset) : save_value(U, NULL);
        struct stack_value call_arg_args =
            argc == 0 ? save_value(U, NULL) : peek_stack_value(U, argc - 1 + offset);

        internal_call(
            U, argc, callable_offset + offset + 1,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );
    }
}

void callstackI_call_from_interpreter(
    morphine_coroutine_t U,
    struct value *callable,
    struct value *self,
    ml_size argc
) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    struct function *function = valueI_as_function_or_error(U->I, *callinfo->s.stack.source);

    if (argc > function->params_count) {
        throwI_error(U->I, "arguments count is greater than params count");
    }

    struct stack_value saved_callable = save_value(U, callable);
    struct stack_value saved_self;
    if (self != NULL) {
        saved_self = save_value(U, self);
    } else {
        saved_self = save_value(U, callinfo->s.direct.self);
    }

    struct value mt_field;
    if (mm_unlikely(metatableI_builtin_test(U->I, *callable, MORPHINE_METAFIELD_CALL, &mt_field))) {
        gcI_safe_enter(U->I);
        struct vector *vector = gcI_safe_obj(U->I, vector, vectorI_create(U->I, argc));

        for (ml_size i = 0; i < argc; i++) {
            struct value arg = callinfo->s.stack.space[function->slots_count + i];
            vectorI_set(U->I, vector, i, arg);
        }

        stackI_push(U, mt_field);
        stackI_push(U, *extract_value(U, saved_callable));
        stackI_push(U, *extract_value(U, saved_self));
        stackI_push(U, valueI_object(vector));

        struct stack_value call_arg_callable = peek_stack_value(U, 3);
        struct stack_value call_arg_env = save_value(U, NULL);
        struct stack_value call_arg_self = peek_stack_value(U, 2);
        struct stack_value call_arg_args = peek_stack_value(U, 1);

        internal_call(
            U, 2, 4,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );

        gcI_safe_exit(U->I);
    } else {
        struct stack_value call_arg_callable = saved_callable;
        struct stack_value call_arg_env = save_value(U, NULL);
        struct stack_value call_arg_self = saved_self;
        struct stack_value call_arg_args =
            argc == 0 ? save_value(U, NULL) : save_value(U, callinfo->s.stack.space + function->slots_count);

        internal_call(
            U, argc, 0,
            call_arg_callable,
            call_arg_env,
            call_arg_self,
            call_arg_args
        );
    }
}
