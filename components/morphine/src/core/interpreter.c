//
// Created by whyiskra on 15.11.23.
//

#include "morphine/core/interpreter.h"
#include "morphine/core/instance.h"
#include "morphine/core/operations.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/gc/control.h"
#include "morphine/gc/finalizer.h"

#define semicolon_blk(x) do { x } while (0)

// loop

#define sp_fetch() \
    semicolon_blk( \
        if (unlikely(*position >= instructions_count)) { \
            callstackI_return(U, valueI_nil); \
            sp_yield(); \
        } \
        instruction = F->instructions[*position]; \
    )

#define sp_yield() semicolon_blk(return;)
#define sp_next()  semicolon_blk((*position)++;)
#define sp_end()   sp_next(); sp_continue()

#define sp_dispatch(o) switch (o)
#define sp_case(o)     case (o):
#define sp_continue()  break;

// access

#define complex_fun(name, s, ...) \
    semicolon_blk( \
        op_result_t operation_result = name(U, s, __VA_ARGS__, 0, true); \
        if(unlikely(operation_result != NORMAL)) { \
            if (operation_result == CALLED) { \
                sp_yield(); \
            } else if(operation_result == CALLED_COMPLETE) { \
                callstackI_continue(U, 0); \
            } \
        } \
    )

#define get_raw_slot(C, a) ((C)->s.stack.space + (a))

#define set_slot(C, a, x)  semicolon_blk(struct value _temp = (x); (C)->s.stack.space[(a)] = _temp;)
#define get_slot(C, a, n)  struct value n = ((C)->s.stack.space[(a)]); semicolon_blk()
#define set_param(C, a, x) semicolon_blk(struct value _temp = (x); (C)->s.stack.space[slots_count + (a)] = _temp;)
#define get_param(C, a, n) struct value n = ((C)->s.stack.space[slots_count + (a)]); semicolon_blk()

#define arg1 (instruction.argument1)
#define arg2 (instruction.argument2)
#define arg3 (instruction.argument3)

// other

static inline void clear_params(struct callinfo *C, ml_size count, size_t slots_count) {
    for (ml_size i = 0; i < count; i++) {
        set_param(C, i, valueI_nil);
    }
}

// code

static void step_function(morphine_coroutine_t U, struct function *F) {
#ifdef MORPHINE_ENABLE_JUMPTABLE

#include "jumptable.h"

#endif

    struct callinfo *C = callstackI_info(U);
    size_t *position = &C->pc.position;
    size_t instructions_count = F->instructions_count;
    size_t slots_count = F->slots_count;

    for (;;) {
        morphine_instruction_t instruction;

        sp_fetch();

        sp_dispatch (instruction.opcode) {
sp_case(MORPHINE_OPCODE_YIELD)
            {
                sp_next();
                sp_yield();
            }
sp_case(MORPHINE_OPCODE_LOAD)
            {
                set_slot(C, arg2, F->constants[arg1]);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_MOVE)
            {
                get_slot(C, arg1, value);
                set_slot(C, arg2, value);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_PARAM)
            {
                get_slot(C, arg1, value);
                set_param(C, arg2, value);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ARG)
            {
                if (arg1 < C->info.arguments_count) {
                    set_slot(C, arg2, C->s.direct.args[arg1]);
                } else {
                    set_slot(C, arg2, valueI_nil);
                }
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ENV)
            {
                set_slot(C, arg1, *C->s.direct.env);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_SELF)
            {
                set_slot(C, arg1, *C->s.direct.self);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_INVOKED)
            {
                set_slot(C, arg1, *C->s.direct.callable);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_VECTOR)
            {
                set_slot(C, arg1, valueI_object(vectorI_create(U->I, arg2)));
                sp_end();
            }
sp_case(MORPHINE_OPCODE_TABLE)
            {
                set_slot(C, arg1, valueI_object(tableI_create(U->I)));
                sp_end();
            }
sp_case(MORPHINE_OPCODE_GET)
            {
                get_slot(C, arg1, container);
                get_slot(C, arg2, key);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_get, 1, container, key, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_SET)
            {
                get_slot(C, arg1, container);
                get_slot(C, arg2, key);
                get_slot(C, arg3, value);

                complex_fun(interpreter_fun_set, 1, container, key, value);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ITERATOR)
            {
                get_slot(C, arg1, container);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator, 1, container, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ITERATOR_INIT)
            {
                get_slot(C, arg1, iterator);
                get_slot(C, arg2, key_name);
                get_slot(C, arg3, value_name);

                complex_fun(interpreter_fun_iterator_init, 1, iterator, key_name, value_name);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ITERATOR_HAS)
            {
                get_slot(C, arg1, iterator);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_has, 1, iterator, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ITERATOR_NEXT)
            {
                get_slot(C, arg1, iterator);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_next, 1, iterator, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_JUMP)
            {
                *position = arg1;
                sp_continue();
            }
sp_case(MORPHINE_OPCODE_JUMP_IF)
            {
                get_slot(C, arg1, cond);

                if (convertI_to_boolean(cond)) {
                    *position = arg2;
                } else {
                    *position = arg3;
                }

                sp_continue();
            }
sp_case(MORPHINE_OPCODE_GET_STATIC)
            {
                get_slot(C, arg1, callable);
                struct value extracted = callstackI_extract_callable(U->I, callable);
                struct function *function = valueI_as_function_or_error(U->I, extracted);
                struct value result = functionI_static_get(
                    U->I,
                    function,
                    arg2
                );
                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_SET_STATIC)
            {
                get_slot(C, arg1, callable);
                struct value extracted = callstackI_extract_callable(U->I, callable);
                struct function *function = valueI_as_function_or_error(U->I, extracted);
                get_slot(C, arg3, value);
                functionI_static_set(
                    U->I,
                    function,
                    arg2,
                    value
                );
                sp_end();
            }
sp_case(MORPHINE_OPCODE_GET_CLOSURE)
            {
                get_slot(C, arg1, callable);
                struct closure *closure = valueI_as_closure_or_error(U->I, callable);
                struct value result = closureI_get(
                    U->I,
                    closure,
                    arg2
                );
                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_SET_CLOSURE)
            {
                get_slot(C, arg1, callable);
                struct closure *closure = valueI_as_closure_or_error(U->I, callable);
                get_slot(C, arg3, value);
                closureI_set(
                    U->I,
                    closure,
                    arg2,
                    value
                );
                sp_end();
            }
sp_case(MORPHINE_OPCODE_CLOSURE)
            {
                get_slot(C, arg1, callable);
                ml_size count = arg2;
                struct closure *closure = closureI_create(U->I, callable, count);
                set_slot(C, arg3, valueI_object(closure));
                sp_end();
            }
sp_case(MORPHINE_OPCODE_CALL)
            {
                ml_size count = arg2;

                if (callstackI_state(U) == 1) {
                    callstackI_continue(U, 0);
                    clear_params(C, count, slots_count);
                    sp_end();
                }

                callstackI_continue(U, 1);

                callstackI_call_from_interpreter(
                    U,
                    get_raw_slot(C, arg1),
                    NULL,
                    count
                );

                sp_yield();
            }
sp_case(MORPHINE_OPCODE_SCALL)
            {
                ml_size count = arg2;

                if (callstackI_state(U) == 1) {
                    callstackI_continue(U, 0);
                    clear_params(C, count, slots_count);
                    sp_end();
                }

                callstackI_continue(U, 1);

                callstackI_call_from_interpreter(
                    U,
                    get_raw_slot(C, arg1),
                    get_raw_slot(C, arg3),
                    count
                );

                sp_yield();
            }
sp_case(MORPHINE_OPCODE_LEAVE)
            {
                callstackI_return(U, valueI_nil);
                (*position)++;
                sp_yield();
            }
sp_case(MORPHINE_OPCODE_RETURN)
            {
                get_slot(C, arg1, value);
                callstackI_return(U, value);
                (*position)++;
                sp_yield();
            }
sp_case(MORPHINE_OPCODE_RESULT)
            {
                struct value value = callstackI_result(U);
                set_slot(C, arg1, value);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_ADD)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_add, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_SUB)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_sub, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_MUL)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mul, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_DIV)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_div, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_MOD)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mod, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_EQUAL)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_equal, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_LESS)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_less, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_AND)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_and, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_OR)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_or, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_CONCAT)
            {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_concat, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_TYPE)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_type, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_NEGATIVE)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_negative, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_NOT)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_not, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_REF)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_ref, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_DEREF)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_deref, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
sp_case(MORPHINE_OPCODE_LENGTH)
            {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_length, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
        }
    }
}

static inline void step(morphine_coroutine_t U) {
    morphine_instance_t I = U->I;
    struct callinfo *callinfo = callstackI_info(U);

    if (unlikely(callinfo == NULL)) {
        coroutineI_kill(U);
        return;
    }

    struct value source = *callinfo->s.stack.source;

    I->throw.context = U;

    if (likely(valueI_is_function(source))) {
        callinfo->exit = false;
        step_function(U, valueI_as_function(source));
    } else if (valueI_is_native(source)) {
        callinfo->exit = true;
        struct native *native = valueI_as_native(source);
        native->function(U);
    } else {
        throwI_error(U->I, "attempt to execute unsupported callable");
    }

    I->throw.context = NULL;

    if (unlikely(callinfo->exit && callinfo == callstackI_info(U))) {
        callstackI_pop(U);
    }
}

static inline bool execute_step(morphine_instance_t I) {
    struct interpreter *interpreter = &I->interpreter;

    if (unlikely(gcI_finalize_need(I))) {
        gcI_finalize(I);
    }

    if (unlikely(interpreter->coroutines == NULL)) {
        interpreter->running = NULL;
        interpreter->next = NULL;
        gcI_full(I, 0);

        return gcI_finalize_need(I);
    }

    if (likely(interpreter->running == NULL)) {
        interpreter->running = interpreter->coroutines;
        interpreter->next = interpreter->running->prev;
        interpreter->circle++;
    } else {
        interpreter->next = interpreter->running->prev;
    }

    morphine_coroutine_t coroutine = interpreter->running;

    bool is_current_circle = (interpreter->circle % coroutine->priority) == 0;

    if (likely(is_current_circle && (coroutine->status == COROUTINE_STATUS_RUNNING))) {
        step(coroutine);
    }

    interpreter->running = interpreter->next;
    interpreter->next = NULL;

    return true;
}

struct wrapper_execute_data {
    morphine_instance_t I;
    bool result;
};

static void wrapper_execute_step(void *pointer) {
    struct wrapper_execute_data *data = pointer;
    data->result = execute_step(data->I);
}

static void wrapper_execute(void *pointer) {
    struct wrapper_execute_data *data = pointer;
    while (execute_step(data->I)) { }
    data->result = false;
}

static void wrapper_handler(void *I) {
    throwI_handler(I);
}

void interpreterI_run(morphine_instance_t I) {
    struct wrapper_execute_data data = {
        .I = I,
        .result = true
    };

    while (data.result) {
        throwI_protect(I, wrapper_execute, wrapper_handler, &data, I, false);
    }
}

bool interpreterI_step(morphine_instance_t I) {
    struct wrapper_execute_data data = {
        .I = I,
        .result = true
    };

    throwI_protect(I, wrapper_execute_step, wrapper_handler, &data, I, false);

    return data.result;
}

struct interpreter interpreterI_prototype(void) {
    return (struct interpreter) {
        .coroutines = NULL,
        .running = NULL,
        .next = NULL,
        .circle = 0,
    };
}
