//
// Created by whyiskra on 15.11.23.
//

#include "morphine/core/interpreter.h"
#include "morphine/core/instance.h"
#include "morphine/core/operations.h"
#include "morphine/gc/control.h"
#include "morphine/gc/finalizer.h"
#include "morphine/object/closure.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/utils/assert.h"

#define semicolon_blk(x) do { x } while (0)

// loop

#define sp_fetch() \
    semicolon_blk( \
        if (mm_unlikely(*position >= instructions_count)) { \
            callstackI_pop(U, valueI_nil); \
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
        if(mm_unlikely(operation_result != NORMAL)) { \
            if (operation_result == CALLED) { \
                sp_yield(); \
            } else if(operation_result == CALLED_COMPLETE) { \
                callstackI_continue(U, 0); \
            } \
        } \
    )

#define get_raw_slot(C, a) ((C)->s.stack.base + (a))
#define set_slot(C, a, x)  semicolon_blk(struct value _temp = (x); (C)->s.stack.base[(a)] = _temp;)
#define get_slot(C, a, n)  struct value n = ((C)->s.stack.base[(a)]); semicolon_blk()

#define get_raw_param(C, a) ((C)->s.stack.base + slots_count + (a))
#define set_param(C, a, x) semicolon_blk(struct value _temp = (x); (C)->s.stack.base[slots_count + (a)] = _temp;)
#define get_param(C, a, n) struct value n = ((C)->s.stack.base[slots_count + (a)]); semicolon_blk()

#define arg1 (instruction.argument1)
#define arg2 (instruction.argument2)
#define arg3 (instruction.argument3)

// code

static void step_function(morphine_coroutine_t U, struct function *F) {
#ifdef MORPHINE_ENABLE_JUMPTABLE

    #include "jumptable.h"

#endif

    struct callframe *C = U->callstack.frame;
    ml_size *position = &C->pc.position;
    ml_size instructions_count = F->instructions_count;
    ml_size slots_count = F->slots_count;

    for (;;) {
        morphine_instruction_t instruction;

        sp_fetch();

        sp_dispatch(instruction.opcode) {
            sp_case(MORPHINE_OPCODE_NO_OPERATION) {
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_YIELD) {
                sp_next();
                sp_yield();
            }
            sp_case(MORPHINE_OPCODE_LOAD) {
                set_slot(C, arg2, F->constants[arg1]);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_MOVE) {
                get_slot(C, arg1, value);
                set_slot(C, arg2, value);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_PARAM) {
                get_slot(C, arg1, value);
                set_param(C, arg2, value);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ARG) {
                if (arg1 < C->params.arguments_count) {
                    set_slot(C, arg2, C->s.direct.args[arg1]);
                } else {
                    set_slot(C, arg2, valueI_nil);
                }
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ENV) {
                set_slot(C, arg1, U->env);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_INVOKED) {
                set_slot(C, arg1, *C->s.direct.callable);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_VECTOR) {
                set_slot(C, arg1, valueI_object(vectorI_create(U->I, arg2)));
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_TABLE) {
                set_slot(C, arg1, valueI_object(tableI_create(U->I)));
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_GET) {
                get_slot(C, arg1, container);
                get_slot(C, arg2, key);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_get, 1, container, key, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_SET) {
                get_slot(C, arg1, container);
                get_slot(C, arg2, key);
                get_slot(C, arg3, value);

                complex_fun(interpreter_fun_set, 1, container, key, value);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ITERATOR) {
                get_slot(C, arg1, container);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator, 1, container, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ITERATOR_INIT) {
                get_slot(C, arg1, iterator);
                get_slot(C, arg2, key_name);
                get_slot(C, arg3, value_name);

                complex_fun(interpreter_fun_iterator_init, 1, iterator, key_name, value_name);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ITERATOR_HAS) {
                get_slot(C, arg1, iterator);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_has, 1, iterator, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_ITERATOR_NEXT) {
                get_slot(C, arg1, iterator);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_next, 1, iterator, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_JUMP) {
                *position = arg1;
                sp_continue();
            }
            sp_case(MORPHINE_OPCODE_JUMP_IF) {
                get_slot(C, arg1, cond);

                if (convertI_to_boolean(cond)) {
                    *position = arg2;
                } else {
                    *position = arg3;
                }

                sp_continue();
            }
            sp_case(MORPHINE_OPCODE_CLOSURE) {
                get_slot(C, arg1, callable);
                get_slot(C, arg2, value);
                struct closure *closure = closureI_create(U->I, callable, value);
                set_slot(C, arg3, valueI_object(closure));
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_CLOSURE_VALUE) {
                get_slot(C, arg1, callable);
                struct closure *closure = valueI_as_closure_or_error(U->I, callable);
                struct value result = closureI_value(U->I, closure);
                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_CALL) {
                ml_size count = arg2;

                if (callstackI_state(U) == 1) {
                    callstackI_continue(U, 0);
                    for (ml_size i = 0; i < count; i++) {
                        set_param(C, i, valueI_nil);
                    }
                    struct value value = callstackI_result(U);
                    set_slot(C, arg3, value);
                    sp_end();
                }

                callstackI_continue(U, 1);
                callstackI_call(U, get_raw_slot(C, arg1), get_raw_param(C, 0), count, 0);
                sp_yield();
            }
            sp_case(MORPHINE_OPCODE_RETURN) {
                get_slot(C, arg1, value);
                sp_next();
                callstackI_pop(U, value);
                sp_yield();
            }
            sp_case(MORPHINE_OPCODE_ADD) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_add, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_SUB) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_sub, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_MUL) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mul, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_DIV) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_div, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_MOD) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mod, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_EQUAL) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_equal, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_LESS) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_less, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_AND) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_and, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_OR) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_or, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_CONCAT) {
                get_slot(C, arg1, a);
                get_slot(C, arg2, b);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_concat, 1, a, b, &result);

                set_slot(C, arg3, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_TYPE) {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_type, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_NEGATIVE) {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_negative, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_NOT) {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_not, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_REF) {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_ref, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_DEREF) {
                get_slot(C, arg1, a);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_deref, 1, a, &result);

                set_slot(C, arg2, result);
                sp_end();
            }
            sp_case(MORPHINE_OPCODE_LENGTH) {
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

    struct callframe *frame = U->callstack.frame;
    if (mm_unlikely(frame == NULL)) {
        coroutineI_kill(U);
        return;
    }

    struct value source = callstackI_extract_callable(I, *frame->s.direct.callable);

    callstackI_update_context(U);
    I->interpreter.context = U;
    if (mm_likely(valueI_is_function(source))) {
        step_function(U, valueI_as_function(source));
    } else if (valueI_is_native(source)) {
        struct native *native = valueI_as_native(source);
        native->function(U);
    } else {
        mm_assert_error(U->I, "attempt to execute unsupported value");
    }
    I->interpreter.context = NULL;
}

static inline bool execute_step(morphine_instance_t I) {
    struct interpreter *interpreter = &I->interpreter;

    if (mm_unlikely(gcI_finalize_need(I))) {
        gcI_finalize(I);
    }

    if (mm_unlikely(interpreter->coroutines == NULL)) {
        interpreter->running = NULL;
        interpreter->next = NULL;
        gcI_full(I);

        return gcI_finalize_need(I);
    }

    if (mm_likely(interpreter->running == NULL)) {
        interpreter->running = interpreter->coroutines;
        interpreter->next = interpreter->running->prev;
    } else {
        interpreter->next = interpreter->running->prev;
    }

    morphine_coroutine_t coroutine = interpreter->running;

    if (mm_likely(coroutine->status == COROUTINE_STATUS_RUNNING)) {
        step(coroutine);
    } else if (mm_unlikely(coroutine->status == COROUTINE_STATUS_KILLING)) {
        coroutineI_detach(coroutine);
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
#ifdef MORPHINE_ENABLE_INTERPRETER_YIELD
    morphine_yield_t yield = data->I->platform.yield;
    do {
        if (yield != NULL) {
            yield(data->I->data);
        }
    }
#endif
    while (execute_step(data->I));
    data->result = false;
}

static morphine_catch_result_t wrapper_handler(void *I) {
    return throwI_interpreter_handler(I);
}

struct interpreter interpreterI_prototype(void) {
    return (struct interpreter) {
        .entered = false,
        .context = NULL,
        .coroutines = NULL,
        .running = NULL,
        .next = NULL,
    };
}

void interpreterI_run(morphine_instance_t I) {
    if (I->interpreter.entered) {
        throwI_error(I, "attempt to invoke nested interpreter");
    }

    I->interpreter.entered = true;

    struct wrapper_execute_data data = {
        .I = I,
        .result = true,
    };

    while (data.result) {
        throwI_protect(I, wrapper_execute, wrapper_handler, &data, I);
    }

    I->interpreter.entered = false;
}

bool interpreterI_step(morphine_instance_t I) {
    if (I->interpreter.entered) {
        throwI_error(I, "attempt to invoke nested interpreter");
    }

    I->interpreter.entered = true;

    struct wrapper_execute_data data = {
        .I = I,
        .result = true,
    };

    throwI_protect(I, wrapper_execute_step, wrapper_handler, &data, I);

    I->interpreter.entered = false;

    return data.result;
}
