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
#include "morphine/gc/safe.h"

static void throwI_stacktrace(morphine_state_t S, const char *message) {
    FILE *file = S->I->platform.io.stacktrace;

    fprintf(file, "morphine error (in state %p): %s\n", S, message);
    fprintf(file, "Tracing callstack:\n");

    size_t callstack_size = S->stack.callstack_size;
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
        }

next:
        callstack_index++;
        callstackI_pop(S);
    }
}

struct throw throwI_prototype(void) {
    return (struct throw) {
        .inited = false,
        .is_message = false,
        .error.value = valueI_nil,
        .context_state = NULL
    };
}

void throwI_handler(morphine_instance_t I) {
    struct throw *throw = &I->E.throw;
    I->E.throw.inited = false;

    morphine_state_t state = throw->context_state;

    if (state == NULL) {
        I->platform.functions.signal(I);
    } else {
        throw->context_state = NULL;
    }

    struct callinfo *callinfo = state->stack.callstack;

    {
        while (callinfo != NULL) {
            if (callinfo->catch.enable) {
                break;
            }

            callinfo = callinfo->prev;
        }
    }

    if (callinfo != NULL) {
        // set state
        callinfo->pc.state = callinfo->catch.state;
        callinfo->catch.enable = false;

        // pop while catch
        while (callstackI_info(state) != callinfo) {
            callstackI_pop(state);
        }

        // set error value
        if (throw->is_message) {
            *callinfo->s.thrown.p = valueI_object(stringI_create(I, throw->error.message));
        } else {
            *callinfo->s.thrown.p = throw->error.value;
        }

        size_t stack_size = stackI_space_size(state);
        size_t expected_stack_size = callinfo->catch.space_size;
        if (stack_size > expected_stack_size) {
            stackI_pop(state, stack_size - expected_stack_size);
        }
    } else {
        throwI_stacktrace(state, throwI_message(I));
        stateI_kill_regardless(state);
    }

    gcI_reset_safe(I);
}

morphine_noret void throwI_error(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->E.throw;

    if (throw->inited) {
        throw->is_message = true;
        throw->error.message = message;

        longjmp(throw->handler, 1);
    }

    throwI_panic(I, message);
}

morphine_noret void throwI_panic(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->E.throw;

    throw->is_message = true;
    throw->error.message = message;
    I->platform.functions.signal(I);
}

morphine_noret void throwI_errorv(morphine_instance_t I, struct value value) {
    struct throw *throw = &I->E.throw;

    if (throw->inited) {
        throw->is_message = false;
        throw->error.value = value;

        longjmp(throw->handler, 1);
    }

    throwI_panicv(I, value);
}

morphine_noret void throwI_panicv(morphine_instance_t I, struct value value) {
    struct throw *throw = &I->E.throw;

    throw->is_message = false;
    throw->error.value = value;
    I->platform.functions.signal(I);
}

const char *throwI_message(morphine_instance_t I) {
    struct throw *throw = &I->E.throw;

    if (throw->is_message) {
        return throw->error.message;
    }

    struct string *string = valueI_safe_as_string(throw->error.value, NULL);

    if (string == NULL) {
        return "(Unsupported error value)";
    } else {
        return string->chars;
    }
}

void throwI_catchable(morphine_state_t S, size_t callstate) {
    struct callinfo *callinfo = callstackI_info(S);

    callinfo->catch.enable = true;
    callinfo->catch.state = callstate;
    callinfo->catch.space_size = stackI_space_size(S);
}
