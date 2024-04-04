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
#include "morphine/object/coroutine/stack/call.h"

// loop

#define sp_fetch() \
    morphinem_blk_start \
    if (unlikely(*position >= instructions_count)) { \
        callstackI_return(U, valueI_nil); \
        sp_yield(); \
    } \
    instruction = F->instructions[*position]; \
    morphinem_blk_end

#define sp_yield() morphinem_blk_start return; morphinem_blk_end
#define sp_next() morphinem_blk_start (*position)++; morphinem_blk_end
#define sp_end() sp_next(); sp_continue()

#define sp_dispatch(o) switch (o)
#define sp_case(o) case (o):
#define sp_continue() morphinem_blk_start break; morphinem_blk_end

// access

#define complex_fun(name, s, ...) \
    morphinem_blk_start \
        op_result_t operation_result = name(U, s, __VA_ARGS__, 0, true); \
        if(unlikely(operation_result != NORMAL)) { \
            if (operation_result == CALLED) { \
                sp_yield(); \
            } else if(operation_result == CALLED_COMPLETE) { \
                callstackI_continue(U, 0); \
            } \
        } \
    morphinem_blk_end

#define slot(C, a) ((C)->s.slots.p[(a).value])
#define param(C, a) ((C)->s.params.p[(a)])

#define arg1 instruction.argument1
#define arg2 instruction.argument2
#define arg3 instruction.argument3

// other

static inline void clear_params(struct callinfo *C, size_t count) {
    for (size_t i = 0; i < count; i++) {
        param(C, i) = valueI_nil;
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

    for (;;) {
        instruction_t instruction;

        sp_fetch();

        sp_dispatch (instruction.opcode)
        {
sp_case(OPCODE_YIELD)
            {
                sp_next();
                sp_yield();
            }
sp_case(OPCODE_LOAD)
            {
                slot(C, arg2) = F->constants[arg1.value];
                sp_end();
            }
sp_case(OPCODE_MOVE)
            {
                slot(C, arg2) = slot(C, arg1);
                sp_end();
            }
sp_case(OPCODE_PARAM)
            {
                param(C, arg2.value) = slot(C, arg1);
                sp_end();
            }
sp_case(OPCODE_ARG)
            {
                slot(C, arg2) = C->s.args.p[arg1.value];
                sp_end();
            }
sp_case(OPCODE_ENV)
            {
                slot(C, arg1) = *C->s.env.p;
                sp_end();
            }
sp_case(OPCODE_SELF)
            {
                slot(C, arg1) = *C->s.self.p;
                sp_end();
            }
sp_case(OPCODE_RECURSION)
            {
                slot(C, arg1) = *C->s.callable.p;
                sp_end();
            }
sp_case(OPCODE_TABLE)
            {
                slot(C, arg1) = valueI_object(tableI_create(U->I));
                sp_end();
            }
sp_case(OPCODE_GET)
            {
                struct value container = slot(C, arg1);
                struct value key = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_get, 1, container, key, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_SET)
            {
                struct value container = slot(C, arg1);
                struct value key = slot(C, arg2);
                struct value value = slot(C, arg3);

                complex_fun(interpreter_fun_set, 1, container, key, value);
                sp_end();
            }
sp_case(OPCODE_ITERATOR)
            {
                struct value container = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator, 1, container, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_ITERATOR_INIT)
            {
                struct value iterator = slot(C, arg1);
                complex_fun(interpreter_fun_iterator_init, 1, iterator);
                sp_end();
            }
sp_case(OPCODE_ITERATOR_HAS)
            {
                struct value iterator = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_has, 1, iterator, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_ITERATOR_NEXT)
            {
                struct value iterator = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_iterator_next, 1, iterator, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_JUMP)
            {
                *position = arg1.value;
                sp_continue();
            }
sp_case(OPCODE_JUMP_IF)
            {
                struct value cond = slot(C, arg1);

                if (valueI_as_boolean_or_error(U->I, cond)) {
                    *position = arg2.value;
                } else {
                    *position = arg3.value;
                }

                sp_continue();
            }
sp_case(OPCODE_GET_STATIC)
            {
                slot(C, arg2) = functionI_static_get(U->I, F, arg1.value);
                sp_end();
            }
sp_case(OPCODE_SET_STATIC)
            {
                functionI_static_set(U->I, F, arg1.value, slot(C, arg2));
                sp_end();
            }
sp_case(OPCODE_GET_CLOSURE)
            {
                slot(C, arg2) = closureI_get(U->I, valueI_as_closure_or_error(U->I, *C->s.callable.p), arg1.value);
                sp_end();
            }
sp_case(OPCODE_SET_CLOSURE)
            {
                closureI_set(U->I, valueI_as_closure_or_error(U->I, *C->s.callable.p), arg1.value, slot(C, arg2));
                sp_end();
            }
sp_case(OPCODE_CLOSURE)
            {
                size_t count = arg2.value;
                struct value callable = slot(C, arg1);

                struct closure *closure = closureI_create(U->I, callable, count);

                struct value *params = &param(C, 0);
                struct value result = valueI_object(closure);

                for (size_t i = 0; i < count; i++) {
                    closureI_set(U->I, closure, i, params[i]);
                }

                slot(C, arg3) = result;

                clear_params(C, count);
                sp_end();
            }
sp_case(OPCODE_CALL)
            {
                if (callstackI_state(U) == 1) {
                    callstackI_continue(U, 0);
                    sp_end();
                }

                struct value callable = slot(C, arg1);
                size_t count = arg2.value;

                callstackI_continue(U, 1);

                callstackI_params(
                    U,
                    callable,
                    valueI_nil,
                    count,
                    0
                );

                clear_params(C, 0);
                sp_yield();
            }
sp_case(OPCODE_SCALL)
            {
                if (callstackI_state(U) == 1) {
                    callstackI_continue(U, 0);
                    sp_end();
                }

                struct value callable = slot(C, arg1);
                struct value self = slot(C, arg3);
                size_t count = arg2.value;

                callstackI_continue(U, 1);

                callstackI_params(
                    U,
                    callable,
                    self,
                    count,
                    0
                );

                clear_params(C, 0);
                sp_yield();
            }
sp_case(OPCODE_LEAVE)
            {
                callstackI_return(U, slot(C, arg1));
                (*position)++;
                sp_yield();
            }
sp_case(OPCODE_RESULT)
            {
                slot(C, arg1) = callstackI_result(U);
                sp_end();
            }
sp_case(OPCODE_ADD)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_add, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_SUB)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_sub, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_MUL)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mul, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_DIV)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_div, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_MOD)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_mod, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_EQUAL)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_equal, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_LESS)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_less, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_LESS_EQUAL)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_less_equal, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_AND)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_and, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_OR)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_or, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_CONCAT)
            {
                struct value a = slot(C, arg1);
                struct value b = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_concat, 1, a, b, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_TYPE)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_type, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_NEGATIVE)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_negative, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_NOT)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_not, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_REF)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_ref, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_DEREF)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_deref, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
sp_case(OPCODE_LENGTH)
            {
                struct value a = slot(C, arg1);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_length, 1, a, &result);

                slot(C, arg2) = result;
                sp_end();
            }
        }
    }
}

static inline void step(morphine_coroutine_t U) {
    struct callinfo *callinfo = callstackI_info(U);

    if (unlikely(callinfo == NULL)) {
        U->status = COROUTINE_STATUS_DEAD;
        return;
    }

    U->I->E.throw.context_coroutine = U;

    struct value source = *callinfo->s.source.p;

    if (likely(valueI_is_function(source))) {
        callinfo->exit = false;
        step_function(U, valueI_as_function(source));
    } else if (valueI_is_native(source)) {
        callinfo->exit = true;
        struct native *native = valueI_as_native(source);
        native->function(U);
    } else {
        throwI_error(U->I, "Attempt to execute unsupported callable");
    }

    if (unlikely(callinfo->exit)) {
        callstackI_pop(U);
    }

    U->I->E.throw.context_coroutine = NULL;
}

static inline void attach_candidates(morphine_instance_t I) {
    if (unlikely(I->E.candidates != NULL)) {
        morphine_coroutine_t coroutine = I->E.candidates;

        if (coroutine->status == COROUTINE_STATUS_ATTACHED) {
            coroutine->status = COROUTINE_STATUS_RUNNING;
        } else {
            throwI_panic(I, "Unexpected candidate coroutine status");
        }

        I->E.candidates = coroutine->prev;
        coroutine->prev = I->E.coroutines;
        I->E.coroutines = coroutine;
    }
}

static inline void execute(morphine_instance_t I) {
    attach_candidates(I);

    morphine_coroutine_t last = NULL;
    morphine_coroutine_t current = I->E.coroutines;

    for (;;) {
        if (unlikely(I->G.finalizer.work && I->G.finalizer.coroutine != NULL)) {
            step(I->G.finalizer.coroutine);

            if (current == NULL) {
                attach_candidates(I);

                if (I->E.coroutines == NULL) {
                    continue;
                }

                current = I->E.coroutines;
            }
        } else if (unlikely(current == NULL)) {
            break;
        }

        bool is_current_circle = (I->E.circle % current->priority) == 0;

        if (likely(is_current_circle && (current->status == COROUTINE_STATUS_RUNNING))) {
            step(current);
        }

        if (unlikely(current->status == COROUTINE_STATUS_DEAD)) {
            if (last == NULL) {
                I->E.coroutines = current->prev;
            } else {
                last->prev = current->prev;
            }
            current->status = COROUTINE_STATUS_DETACHED;
        } else {
            last = current;
        }

        current = current->prev;

        if (likely(current == NULL)) {
            attach_candidates(I);

            last = NULL;
            current = I->E.coroutines;

            if (unlikely(current == NULL)) {
                gcI_full(I);
            }

            I->E.circle++;
        }
    }
}

void interpreterI_run(morphine_instance_t I) {
    if (setjmp(I->E.throw.handler) != 0) {
        throwI_handler(I);
    }

    I->E.throw.inited = true;

    execute(I);

    I->E.throw.inited = false;
}

struct interpreter interpreterI_prototype(void) {
    return (struct interpreter) {
        .coroutines = NULL,
        .candidates = NULL,
        .circle = 0,
        .throw = throwI_prototype(),
    };
}
