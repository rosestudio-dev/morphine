//
// Created by whyiskra on 15.11.23.
//

#include "morphine/core/interpreter.h"
#include "morphine/core/instance.h"
#include "morphine/core/call.h"
#include "morphine/core/operations.h"
#include "morphine/core/hook.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/gc/control.h"

// loop

#define sp_fetch() \
    morphinem_blk_start \
    if (morphinem_unlikely(*position >= instructions_count)) { \
        callI_return(S, valueI_nil); \
        sp_yield(); \
    } \
    instruction = P->instructions[*position]; \
    pdbg_hook_interpreter_step(S->I, S); \
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
        op_result_t operation_result = name(S, s, __VA_ARGS__, 0, true); \
        if(morphinem_unlikely(operation_result != NORMAL)) { \
            if (operation_result == CALLED) { \
                sp_yield(); \
            } else if(operation_result == CALLED_COMPLETE) { \
                callI_continue(S, 0); \
            } \
        } \
    morphinem_blk_end

#define slot(C, a) ((C)->s.values.p[(a).value])
#define param(C, P, a) ((C)->s.values.p[(a) + (P)->slots_count])

#define arg1 instruction.argument1
#define arg2 instruction.argument2
#define arg3 instruction.argument3

// other

static inline void clear_params(struct callinfo *C, struct proto *P, size_t count) {
    for (size_t i = 0; i < count; i++) {
        param(C, P, i) = valueI_nil;
    }
}

// code

static void step_proto(morphine_state_t S, struct proto *P) {
#ifdef MORPHINE_ENABLE_JUMPTABLE

#include "jumptable.h"

#endif

    struct callinfo *C = stackI_callinfo(S);
    size_t *position = &C->pc.position;
    size_t instructions_count = P->instructions_count;

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
                slot(C, arg2) = P->constants[arg1.value];
                sp_end();
            }
sp_case(OPCODE_MOVE)
            {
                slot(C, arg2) = slot(C, arg1);
                sp_end();
            }
sp_case(OPCODE_PARAM)
            {
                param(C, P, arg2.value) = slot(C, arg1);
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
                slot(C, arg1) = valueI_object(tableI_create(S->I, 0));
                sp_end();
            }
sp_case(OPCODE_GET)
            {
                struct value table = slot(C, arg1);
                struct value key = slot(C, arg2);
                struct value result = valueI_nil;

                complex_fun(interpreter_fun_get, 1, table, key, &result);

                slot(C, arg3) = result;
                sp_end();
            }
sp_case(OPCODE_SET)
            {
                struct value table = slot(C, arg1);
                struct value key = slot(C, arg2);
                struct value value = slot(C, arg3);

                complex_fun(interpreter_fun_set, 1, table, key, value);
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

                if (valueI_as_boolean_or_error(S, cond)) {
                    *position = arg2.value;
                } else {
                    *position = arg3.value;
                }

                sp_continue();
            }
sp_case(OPCODE_GET_STATIC)
            {
                slot(C, arg2) = protoI_static_get(S, P, arg1.value);
                sp_end();
            }
sp_case(OPCODE_SET_STATIC)
            {
                protoI_static_set(S, P, arg1.value, slot(C, arg2));
                sp_end();
            }
sp_case(OPCODE_GET_CLOSURE)
            {
                slot(C, arg2) = closureI_get(S, valueI_as_closure_or_error(S, *C->s.callable.p), arg1.value);
                sp_end();
            }
sp_case(OPCODE_SET_CLOSURE)
            {
                closureI_set(S, valueI_as_closure_or_error(S, *C->s.callable.p), arg1.value, slot(C, arg2));
                sp_end();
            }
sp_case(OPCODE_CLOSURE)
            {
                size_t count = arg2.value;
                struct value callable = slot(C, arg1);

                struct closure *closure = closureI_create(S->I, callable, count);

                struct value *params = &param(C, P, 0);
                struct value result = valueI_object(closure);

                for (size_t i = 0; i < count; i++) {
                    closureI_set(S, closure, i, params[i]);
                }

                slot(C, arg3) = result;

                clear_params(C, P, count);
                sp_end();
            }
sp_case(OPCODE_CALL)
            {
                if (callI_callstate(S) == 1) {
                    callI_continue(S, 0);
                    sp_end();
                }

                struct value callable = slot(C, arg1);
                size_t count = arg2.value;

                struct value *params = &param(C, P, 0);

                callI_continue(S, 1);

                callI_do(
                    S,
                    callable,
                    valueI_nil,
                    count,
                    params,
                    0
                );

                clear_params(C, P, 0);
                sp_yield();
            }
sp_case(OPCODE_SCALL)
            {
                if (callI_callstate(S) == 1) {
                    callI_continue(S, 0);
                    sp_end();
                }

                struct value callable = slot(C, arg1);
                struct value self = slot(C, arg3);
                size_t count = arg2.value;

                struct value *params = &param(C, P, 0);

                callI_continue(S, 1);

                callI_do(
                    S,
                    callable,
                    self,
                    count,
                    params,
                    0
                );

                clear_params(C, P, 0);
                sp_yield();
            }
sp_case(OPCODE_LEAVE)
            {
                callI_return(S, slot(C, arg1));
                (*position)++;
                sp_yield();
            }
sp_case(OPCODE_RESULT)
            {
                slot(C, arg1) = callI_result(S);
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

static inline void step(morphine_state_t S) {
    struct callinfo *callinfo = stackI_callinfo(S);

    if (morphinem_unlikely(callinfo == NULL)) {
        S->status = STATE_STATUS_DEAD;
        throwI_message_error(S, "Require callinfo");
    }

    struct value source = *callinfo->s.source.p;

    if (morphinem_likely(valueI_is_proto(source))) {
        callinfo->exit = false;
        step_proto(S, valueI_as_proto(source));
    } else if (valueI_is_native(source)) {
        callinfo->exit = true;
        struct native *native = valueI_as_native(source);
        native->function(S);
    } else {
        throwI_message_error(S, "Attempt to execute unsupported callable");
    }

    if (morphinem_unlikely(callinfo->exit)) {
        stackI_call_pop(S);
    }
}

static inline void attach_candidates(morphine_instance_t I) {
    if (morphinem_unlikely(I->candidates != NULL)) {
        morphine_state_t state = I->candidates;
        I->candidates = state->prev;

        state->prev = I->states;
        I->states = state;

        if (state->status == STATE_STATUS_ATTACHED) {
            state->status = STATE_STATUS_RUNNING;
        }
    }
}

static inline void execute(morphine_instance_t I) {
    attach_candidates(I);

    morphine_state_t last = NULL;
    morphine_state_t current = I->states;

    for (;;) {
        if (morphinem_unlikely(I->G.finalizer.work && I->state_finalizer != NULL)) {
            step(I->state_finalizer);

            if (current == NULL) {
                attach_candidates(I);

                if (I->states == NULL) {
                    continue;
                }

                current = I->states;
            }
        } else if (morphinem_unlikely(current == NULL)) {
            break;
        }

        bool is_current_circle = (I->interpreter_circle % current->settings.priority) == 0;

        if (morphinem_likely(is_current_circle && (current->status == STATE_STATUS_RUNNING))) {
            step(current);
        }

        if (morphinem_unlikely((current->status == STATE_STATUS_DEAD) || (current->status == STATE_STATUS_DETACHED))) {
            if (last == NULL) {
                I->states = current->prev;
            } else {
                last->prev = current->prev;
            }
            current->status = STATE_STATUS_DETACHED;
        } else {
            last = current;
        }

        current = current->prev;

        if (morphinem_likely(current == NULL)) {
            attach_candidates(I);

            last = NULL;
            current = I->states;

            if (morphinem_unlikely(current == NULL)) {
                gcI_full(I);
            }

            I->interpreter_circle++;
        }
    }
}

void interpreterI_run(morphine_instance_t I) {
    if (setjmp(I->throw.handler) != 0) {
        throwI_handler(I);
    }

    I->throw.inited = true;

    execute(I);

    I->throw.inited = false;
}
