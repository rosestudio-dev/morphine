//
// Created by whyiskra on 3/24/24.
//

#include "morphine/stack/call.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/function.h"
#include "morphine/object/closure.h"
#include "morphine/gc/hot.h"
#include "morphine/gc/allocator.h"
#include "morphine/stack/access.h"
#include "morphine/object/table.h"
#include "functions.h"

static inline void stackI_call(morphine_coroutine_t U, struct value callable, struct value self, size_t argc, size_t pop_size) {
    // get source and calc values size

    struct value source = callstackI_extract_callable(U->I, callable);

    size_t values_size = 6 + argc;
    size_t slots_count = 0;
    size_t params_count = 0;

    if (valueI_is_function(source)) {
        struct function *function = valueI_as_function(source);

        if (argc != function->arguments_count) {
            throwI_error(U->I, "Wrong arguments count");
        }

        slots_count = function->slots_count;
        params_count = function->params_count;
    }

    // get env

    struct value env;
    if (callstackI_info(U) != NULL) {
        env = *callstackI_info(U)->s.env.p;
    } else {
        env = valueI_object(U->I->env);
    }

    // create callinfo

    if (pop_size > stackI_space_size(U)) {
        throwI_error(U->I, "Cannot pop values after call");
    }

    stackI_ptr base = stack_raise(U, values_size + slots_count + params_count);

    struct callinfo *callinfo = gcI_hot_callinfo(U->I);
    if (callinfo == NULL) {
        callinfo = allocI_uni(U->I, NULL, sizeof(struct callinfo));
    }

    (*callinfo) = (struct callinfo) {
        .s.base = base,
        .s.source = stack_ptr(base.p + 1),
        .s.env = stack_ptr(base.p + 2),
        .s.self = stack_ptr(base.p + 3),
        .s.result = stack_ptr(base.p + 4),
        .s.thrown = stack_ptr(base.p + 5),
        .s.args = stack_ptr(base.p + 6),
        .s.slots = stack_ptr(base.p + 6 + argc),
        .s.params = stack_ptr(base.p + 6 + argc + slots_count),
        .s.space = stack_ptr(U->stack.allocated + U->stack.top),
        .s.top = stack_ptr(U->stack.allocated + U->stack.top),
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
    U->stack.callstack_size ++;

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

void callstackI_unsafe(
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
        stackI_push(U, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = args[i];

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(U, new_args, 2);
    } else {
        stackI_call(U, callable, self, argc, pop_size);
        stackI_set_args_unsafe(U, args, argc);
    }
}

void callstackI_stack(
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
        stackI_push(U, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = stackI_peek(U, argc - i + offset);

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(U, new_args, 2);
    } else {
        struct callinfo *callinfo = callstackI_info(U);
        stackI_call(U, callable, self, argc, pop_size);

        for (size_t i = 0; i < argc; i++) {
            struct value arg = stack_peek(U, callinfo, argc - i - 1 + offset);
            callstackI_info(U)->s.args.p[i] = arg;
        }
    }
}

void callstackI_params(
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
        stackI_push(U, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(U->I, i);
            struct value arg = callinfo->s.params.p[i];

            tableI_set(U->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(U, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(U, new_args, 2);
        return;
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

    stack_reduce(U, (size_t) (callinfo->s.top.p - callinfo->s.base.p));

    callstackI_info(U) = callinfo->prev;
    U->stack.callstack_size --;

    gcI_dispose_callinfo(U->I, callinfo);

    stackI_pop(U, pop_size);

    if (callstackI_info(U) == NULL) {
        coroutineI_kill(U);
    }
}

void callstackI_info_free(morphine_instance_t I, struct callinfo *callinfo) {
    allocI_free(I, callinfo);
}
