//
// Created by why-iskra on 04.04.2024.
//

#include "morphine/object/coroutine/callstack.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/closure.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/core/value.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/hot.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/misc/metatable.h"

static inline void stackI_call(
    morphine_coroutine_t U, struct value callable, struct value self, size_t argc, size_t pop_size
) {
    morphine_instance_t I = U->I;

    // get source and calc values size

    struct value source = callstackI_extract_callable(I, callable);

    size_t slots_count = 0;
    size_t params_count = 0;

    if (valueI_is_function(source)) {
        struct function *function = valueI_as_function(source);

        if (argc != function->arguments_count) {
            throwI_error(I, "Wrong arguments count");
        }

        slots_count = function->slots_count;
        params_count = function->params_count;
    }

    // checks

    if (unlikely(argc > MLIMIT_CALLABLE_ARGS)) {
        throwI_error(I, "Too many args");
    }

    if (unlikely(params_count > MLIMIT_CALLABLE_PARAMS)) {
        throwI_error(I, "Too many params");
    }

    if (unlikely(slots_count > MLIMIT_CALLABLE_SLOTS)) {
        throwI_error(I, "Too many slots");
    }

    // get env

    struct value env;
    if (callstackI_info(U) != NULL) {
        env = *callstackI_info(U)->s.env.p;
    } else {
        env = U->env;
    }

    // create callinfo

    if (pop_size > stackI_space(U)) {
        throwI_error(I, "Cannot pop values after call");
    }

    size_t raise_size = 6 + argc + slots_count + params_count;
    struct value *base = stackI_raise(U, raise_size);

    struct callinfo *callinfo = gcI_hot_callinfo(I);
    if (callinfo == NULL) {
        callinfo = allocI_uni(I, NULL, sizeof(struct callinfo));
    }

    (*callinfo) = (struct callinfo) {
        .s.base = stackI_ptr(base),
        .s.source = stackI_ptr(base + 1),
        .s.env = stackI_ptr(base + 2),
        .s.self = stackI_ptr(base + 3),
        .s.result = stackI_ptr(base + 4),
        .s.thrown = stackI_ptr(base + 5),
        .s.args = stackI_ptr(base + 6),
        .s.slots = stackI_ptr(base + 6 + argc),
        .s.params = stackI_ptr(base + 6 + argc + slots_count),
        .s.space = stackI_ptr(U->stack.allocated + U->stack.top),
        .s.top = stackI_ptr(U->stack.allocated + U->stack.top),
        .pop_size = pop_size,
        .arguments_count = argc,
        .pc.position = 0,
        .pc.state = 0,
        .catch.enable = false,
        .catch.state = 0,
        .exit = false,
        .prev = callstackI_info(U)
    };

    callstackI_info(U) = callinfo;
    U->callstack.size++;

    // init callinfo stack region

    *callinfo->s.callable.p = callable;
    *callinfo->s.source.p = source;
    *callinfo->s.env.p = env;
    *callinfo->s.self.p = self;
}

static inline struct callinfo *checkargs(morphine_coroutine_t U, size_t argc) {
    struct callinfo *callinfo = callstackI_info_or_error(U);

    if (argc != callinfo->arguments_count) {
        throwI_error(U->I, "Wrong arguments count");
    }

    return callinfo;
}

static inline void stackI_set_args_unsafe(morphine_coroutine_t U, struct value *args, size_t argc) {
    struct callinfo *callinfo = checkargs(U, argc);

    for (size_t i = 0; i < argc; i++) {
        callinfo->s.args.p[i] = args[i];
    }
}

struct callstack callstackI_prototype(void) {
    return (struct callstack) {
        .callinfo = NULL,
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
    callstack->callinfo = NULL;
    callstack->size = 0;
}

void callstackI_callinfo_free(morphine_instance_t I, struct callinfo *callinfo) {
    allocI_free(I, callinfo);
}

void callstackI_call_unsafe(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    struct value *args,
    size_t argc,
    size_t pop_size
) {
    struct value mt_field;
    if (unlikely(metatableI_test(U->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(U->I);
        struct value args_table = valueI_object(table);
        gcI_safe(U->I, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = args[i];

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size);
        stackI_set_args_unsafe(U, new_args, 2);

        gcI_reset_safe(U->I);
    } else {
        stackI_call(U, callable, self, argc, pop_size);
        stackI_set_args_unsafe(U, args, argc);
    }
}

void callstackI_call_stack(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t offset,
    size_t argc,
    size_t pop_size
) {
    struct value mt_field;
    if (unlikely(metatableI_test(U->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(U->I);
        struct value args_table = valueI_object(table);
        gcI_safe(U->I, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = stackI_peek(U, argc - i - 1 + offset);

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size);
        stackI_set_args_unsafe(U, new_args, 2);

        gcI_reset_safe(U->I);
    } else {
        struct callinfo *callinfo = callstackI_info(U);
        stackI_call(U, callable, self, argc, pop_size);

        for (size_t i = 0; i < argc; i++) {
            struct value arg = stackI_callinfo_peek(U, callinfo, argc - i - 1 + offset);
            callstackI_info(U)->s.args.p[i] = arg;
        }
    }
}

void callstackI_call_params(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t argc,
    size_t pop_size
) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    size_t params_count = valueI_as_function_or_error(U->I, *(callinfo->s.source.p))->params_count;

    if (params_count < argc) {
        throwI_error(U->I, "Arguments count is greater than params count");
    }

    struct value mt_field;
    if (unlikely(metatableI_test(U->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(U->I);
        struct value args_table = valueI_object(table);
        gcI_safe(U->I, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = callinfo->s.params.p[i];

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size);
        stackI_set_args_unsafe(U, new_args, 2);

        gcI_reset_safe(U->I);
    } else {
        stackI_call(U, callable, self, argc, pop_size);

        for (size_t i = 0; i < argc; i++) {
            struct value arg = callinfo->s.params.p[i];
            callstackI_info(U)->s.args.p[i] = arg;
        }
    }
}

void callstackI_pop(morphine_coroutine_t U) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    size_t pop_size = callinfo->pop_size;

    stackI_reduce(U, (size_t) (callinfo->s.top.p - callinfo->s.base.p));

    callstackI_info(U) = callinfo->prev;
    U->callstack.size--;

    gcI_dispose_callinfo(U->I, callinfo);

    stackI_pop(U, pop_size);

    if (callstackI_info(U) == NULL) {
        coroutineI_kill(U);
    }
}

struct value callstackI_extract_callable(morphine_instance_t I, struct value callable) {
    size_t counter = 0;
repeat:;
    if (counter > 1000000) {
        throwI_error(I, "Possible recursion while extracting callable");
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

    throwI_error(I, "Cannot extract callable value");
}

struct value callstackI_result(morphine_coroutine_t U) {
    return *callstackI_info_or_error(U)->s.result.p;
}

void callstackI_return(morphine_coroutine_t U, struct value value) {
    struct callinfo *callinfo = callstackI_info_or_error(U);

    if (callinfo->prev != NULL) {
        *callinfo->prev->s.result.p = value;
    }

    callinfo->exit = true;
}

void callstackI_leave(morphine_coroutine_t U) {
    callstackI_return(U, valueI_nil);
}

void callstackI_continue(morphine_coroutine_t U, size_t state) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    callinfo->pc.state = state;
    callinfo->exit = false;
}

size_t callstackI_state(morphine_coroutine_t U) {
    return callstackI_info_or_error(U)->pc.state;
}
