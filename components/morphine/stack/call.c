//
// Created by whyiskra on 3/24/24.
//

#include "morphine/stack/call.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/proto.h"
#include "morphine/object/closure.h"
#include "morphine/gc/hot.h"
#include "morphine/gc/allocator.h"
#include "morphine/stack/access.h"
#include "morphine/object/table.h"
#include "functions.h"

static inline void stackI_call(morphine_state_t S, struct value callable, struct value self, size_t argc, size_t pop_size) {
    // get source and calc values size

    struct value source = callstackI_extract_callable(S->I, callable);

    size_t values_size = 6 + argc;
    size_t slots_count = 0;
    size_t params_count = 0;

    if (valueI_is_proto(source)) {
        struct proto *proto = valueI_as_proto(source);

        if (argc != proto->arguments_count) {
            throwI_error(S->I, "Wrong arguments count");
        }

        slots_count = proto->slots_count;
        params_count = proto->params_count;
    }

    // get env

    struct value env;
    if (callstackI_info(S) != NULL) {
        env = *callstackI_info(S)->s.env.p;
    } else {
        env = valueI_object(S->I->env);
    }

    // create callinfo

    if (pop_size > stackI_space_size(S)) {
        throwI_error(S->I, "Cannot pop values after call");
    }

    stackI_ptr base = stack_raise(S, values_size + slots_count + params_count);

    struct callinfo *callinfo = gcI_hot_callinfo(S->I);
    if (callinfo == NULL) {
        callinfo = allocI_uni(S->I, NULL, sizeof(struct callinfo));
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
        .s.space = stack_ptr(S->stack.allocated + S->stack.top),
        .s.top = stack_ptr(S->stack.allocated + S->stack.top),
        .pop_size = pop_size,
        .arguments_count = argc,
        .pc.position = 0,
        .pc.state = 0,
        .catch.enable = false,
        .catch.state = 0,
        .exit = false,
        .prev = callstackI_info(S)
    };

    callstackI_info(S) = callinfo;
    S->stack.callstack_size ++;

    // init callinfo stack region

    *callinfo->s.callable.p = callable;
    *callinfo->s.source.p = source;
    *callinfo->s.env.p = env;
    *callinfo->s.self.p = self;
}

static inline struct callinfo *checkargs(morphine_state_t S, size_t argc) {
    struct callinfo *callinfo = callstackI_info_or_error(S);

    if (argc != callinfo->arguments_count) {
        throwI_error(S->I, "Wrong arguments count");
    }

    return callinfo;
}

static inline void stackI_set_args_unsafe(morphine_state_t S, struct value *args, size_t argc) {
    struct callinfo *callinfo = checkargs(S, argc);

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

    if (valueI_is_native(callable) || valueI_is_proto(callable)) {
        return callable;
    }

    throwI_error(I, "Cannot extract callable value");
}

void callstackI_unsafe(
    morphine_state_t S,
    struct value callable,
    struct value self,
    struct value *args,
    size_t argc,
    size_t pop_size
) {
    struct value mt_field;
    if (unlikely(metatableI_test(S->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(S->I);
        struct value args_table = valueI_object(table);
        stackI_push(S, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(S->I, i);
            struct value arg = args[i];

            tableI_set(S->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(S, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(S, new_args, 2);
    } else {
        stackI_call(S, callable, self, argc, pop_size);
        stackI_set_args_unsafe(S, args, argc);
    }
}

void callstackI_stack(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t offset,
    size_t argc,
    size_t pop_size
) {
    struct value mt_field;
    if (unlikely(metatableI_test(S->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(S->I);
        struct value args_table = valueI_object(table);
        stackI_push(S, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(S->I, i);
            struct value arg = stackI_peek(S, argc - i + offset);

            tableI_set(S->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(S, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(S, new_args, 2);
    } else {
        struct callinfo *callinfo = callstackI_info(S);
        stackI_call(S, callable, self, argc, pop_size);

        for (size_t i = 0; i < argc; i++) {
            struct value arg = stack_peek(S, callinfo, argc - i - 1 + offset);
            callstackI_info(S)->s.args.p[i] = arg;
        }
    }
}

void callstackI_params(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t argc,
    size_t pop_size
) {
    struct callinfo *callinfo = callstackI_info_or_error(S);
    size_t params_count = valueI_as_proto_or_error(S->I, *(callinfo->s.source.p))->params_count;

    if (params_count < argc) {
        throwI_error(S->I, "Arguments count is greater than params count");
    }

    struct value mt_field;
    if (unlikely(metatableI_test(S->I, callable, MF_CALL, &mt_field))) {
        struct table *table = tableI_create(S->I);
        struct value args_table = valueI_object(table);
        stackI_push(S, args_table);

        for (size_t i = 0; i < argc; i++) {
            struct value key = valueI_size2integer(S->I, i);
            struct value arg = callinfo->s.params.p[i];

            tableI_set(S->I, table, key, arg);
        }

        struct value new_args[2] = { self, args_table };

        stackI_call(S, mt_field, callable, 2, pop_size + 1);
        stackI_set_args_unsafe(S, new_args, 2);
        return;
    } else {
        stackI_call(S, callable, self, argc, pop_size);

        for (size_t i = 0; i < argc; i++) {
            struct value arg = callinfo->s.params.p[i];
            callstackI_info(S)->s.args.p[i] = arg;
        }
    }
}

void callstackI_pop(morphine_state_t S) {
    struct callinfo *callinfo = callstackI_info_or_error(S);
    size_t pop_size = callinfo->pop_size;

    stack_reduce(S, (size_t) (callinfo->s.top.p - callinfo->s.base.p));

    callstackI_info(S) = callinfo->prev;
    S->stack.callstack_size --;

    gcI_dispose_callinfo(S->I, callinfo);

    stackI_pop(S, pop_size);

    if (callstackI_info(S) == NULL) {
        stateI_kill(S);
    }
}

void callstackI_info_free(morphine_instance_t I, struct callinfo *callinfo) {
    allocI_free(I, callinfo);
}
