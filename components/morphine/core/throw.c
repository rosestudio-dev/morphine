//
// Created by whyiskra on 16.12.23.
//

#include <setjmp.h>
#include <stdio.h>
#include <inttypes.h>
#include "morphine/object/state.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/stack/call.h"
#include "morphine/stack/access.h"

static void throwI_stacktrace(morphine_state_t S, const char *message) {
    FILE *file = S->I->platform.io.stacktrace;

    fprintf(file, "morphine error (in state %p): %s\n", S, message);
    fprintf(file, "Tracing callstack:\n");

    size_t callstack_size = 0;

    {
        struct callinfo *callstack = callstackI_info(S);
        while (callstack != NULL) {
            callstack_size++;
            callstack = callstack->prev;
        }
    }

    size_t callstack_index = 0;
    while (callstackI_info(S) != NULL) {
        if (callstack_size >= 30) {
            if (callstack_index == 10) {
                fprintf(file, "    ... (skipped %zu)\n", callstack_size - 20);
            }

            if (callstack_index >= 10 && callstack_index < callstack_size - 10) {
                goto next;
            }
        }

        fprintf(file, "    ");

        struct value callable = *callstackI_info(S)->s.source.p;

        if (valueI_is_proto(callable)) {
            struct proto *proto = valueI_as_proto(callable);

            size_t position = callstackI_info(S)->pc.position;
            uint32_t line = 0;
            if (position < proto->instructions_count) {
                line = proto->instructions[position].line;
            }

            fprintf(file, "[line: %"PRIu32", p: %zu] proto %s (%p)\n", line, position, proto->name, proto);
        } else if (valueI_is_native(callable)) {
            struct native *native = valueI_as_native(callable);

            fprintf(
                file,
                "[s: %zu] native %s (%p)\n",
                callstackI_info(S)->pc.state,
                native->name,
                native->function
            );
        } else {
            throwI_message_panic(S->I, S, "Error printer expects proto or native in callstack");
        }

next:
        callstack_index++;
        callstackI_pop(S);
    }
}

struct throw throwI_prototype(void) {
    return (struct throw) {
        .inited = false,
        .cause_state = NULL,
        .is_message = false,
        .result.value = valueI_nil,
    };
}

void throwI_handler(morphine_instance_t I) {
    I->throw.inited = false;

    morphine_state_t caused_S = I->throw.cause_state;
    struct callinfo *callstack = caused_S->stack.callstack;

    bool catchable = true;

    while (true) {
        if (callstack == NULL) {
            catchable = false;
            break;
        } else {
            if (callstack->catch.enable) {
                callstack->pc.state = callstack->catch.state;
                callstack->catch.enable = false;
                break;
            }

            callstack = callstack->prev;
        }
    }

    if (catchable) {
        struct value *thrown = caused_S->stack.callstack->s.thrown.p;
        if (I->throw.is_message) {
            const char *message = "Empty message";

            if (I->throw.result.message != NULL) {
                message = I->throw.result.message;
            }

            *thrown = valueI_object(stringI_create(I, message));
        } else {
            *thrown = caused_S->I->throw.result.value;
        }

        while (caused_S->stack.callstack != callstack) {
            callstackI_pop(caused_S);
        }

        *caused_S->stack.callstack->s.thrown.p = *thrown;

        size_t stack_size = stackI_space_size(caused_S);
        size_t expected_stack_size = callstack->catch.space_size;
        if (stack_size > expected_stack_size) {
            stackI_pop(caused_S, stack_size - expected_stack_size);
        }
    } else {
        const char *message = throwI_get_panic_message(I);

        if (message == NULL) {
            message = "Empty message";
        }

        throwI_stacktrace(caused_S, message);
        stateI_kill_regardless(caused_S);
    }
}

morphine_noret void throwI_error(morphine_state_t S, struct value value) {
    morphine_instance_t I = S->I;

    if (I->throw.inited) {
        I->throw.cause_state = S;
        I->throw.is_message = false;
        I->throw.result.value = value;

        longjmp(I->throw.handler, 1);
    } else {
        throwI_panic(I, S, value);
    }
}

morphine_noret void throwI_message_error(morphine_state_t S, const char *message) {
    morphine_instance_t I = S->I;

    if (I->throw.inited) {
        I->throw.cause_state = S;
        I->throw.is_message = true;
        I->throw.result.message = message;

        longjmp(I->throw.handler, 1);
    } else {
        throwI_message_panic(I, S, message);
    }
}

morphine_noret void throwI_panic(morphine_instance_t I, morphine_state_t cause_state, struct value value) {
    I->throw.cause_state = cause_state;
    I->throw.is_message = false;
    I->throw.result.value = value;
    I->platform.functions.signal(I);
}

morphine_noret void throwI_message_panic(morphine_instance_t I, morphine_state_t cause_state, const char *message) {
    I->throw.cause_state = cause_state;
    I->throw.is_message = true;
    I->throw.result.message = message;
    I->platform.functions.signal(I);
}

void throwI_catchable(morphine_state_t S, size_t callstate) {
    struct callinfo *callinfo = callstackI_info(S);

    callinfo->catch.enable = true;
    callinfo->catch.state = callstate;
    callinfo->catch.space_size = stackI_space_size(S);
}

const char *throwI_get_panic_message(morphine_instance_t I) {
    const char *message = NULL;
    if (I->throw.is_message) {
        message = I->throw.result.message;
    } else {
        struct value value = valueI_value2string(I, I->throw.result.value);
        message = valueI_as_string(value)->chars;
    }

    return message;
}
