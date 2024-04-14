//
// Created by whyiskra on 16.12.23.
//

#include <setjmp.h>
#include <stdio.h>
#include <inttypes.h>
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"

static void throwI_stacktrace(morphine_coroutine_t U, const char *message) {
    FILE *file = U->I->platform.io.stacktrace;

    fprintf(file, "morphine error (in coroutine %p): %s\n", U, message);
    fprintf(file, "tracing callstack:\n");

    size_t callstack_size = U->callstack.size;
    size_t callstack_index = 0;
    struct callinfo *callinfo = callstackI_info(U);
    while (callinfo != NULL) {
        if (callstack_size >= 30) {
            if (callstack_index == 10) {
                fprintf(file, "    ... (skipped %zu)\n", callstack_size - 20);
            }

            if (callstack_index >= 10 && callstack_index < callstack_size - 10) {
                goto next;
            }
        }

        fprintf(file, "    ");

        struct value callable = *callinfo->s.source.p;

        if (valueI_is_function(callable)) {
            struct function *function = valueI_as_function(callable);

            size_t position = callinfo->pc.position;
            uint32_t line = 0;
            if (position < function->instructions_count) {
                line = function->instructions[position].line;
            }

            fprintf(file, "[line: %"PRIu32", p: %zu] function %s (%p)\n", line, position, function->name, function);
        } else if (valueI_is_native(callable)) {
            struct native *native = valueI_as_native(callable);

            fprintf(
                file,
                "[s: %zu] native %s (%p)\n",
                callinfo->pc.state,
                native->name,
                native->function
            );
        }

next:
        callstack_index++;
        callinfo = callinfo->prev;
    }
}

struct throw throwI_prototype(void) {
    return (struct throw) {
        .inited = false,
        .is_message = false,
        .error.value = valueI_nil,
        .context_coroutine = NULL
    };
}

void throwI_handler(morphine_instance_t I) {
    struct throw *throw = &I->E.throw;
    I->E.throw.inited = false;

    morphine_coroutine_t coroutine = throw->context_coroutine;

    if (coroutine == NULL) {
        I->platform.functions.signal(I);
    } else {
        throw->context_coroutine = NULL;
    }

    struct callinfo *callinfo = callstackI_info(coroutine);

    {
        while (callinfo != NULL) {
            if (callinfo->catch.enable) {
                break;
            }

            callinfo = callinfo->prev;
        }
    }

    callstackI_fix_uninit(coroutine);

    if (callinfo != NULL) {
        // set state
        callinfo->pc.state = callinfo->catch.state;
        callinfo->catch.enable = false;

        // pop while catch
        while (callstackI_info(coroutine) != callinfo) {
            callstackI_pop(coroutine);
        }

        // set error value
        if (throw->is_message) {
            *callinfo->s.thrown.p = valueI_object(stringI_create(I, throw->error.message));
        } else {
            *callinfo->s.thrown.p = throw->error.value;
        }

        size_t stack_size = stackI_space(coroutine);
        size_t expected_stack_size = callinfo->catch.space_size;
        if (stack_size > expected_stack_size) {
            stackI_pop(coroutine, stack_size - expected_stack_size);
        }
    } else {
        throwI_stacktrace(coroutine, throwI_message(I));

        while (callstackI_info(coroutine) != NULL) {
            callstackI_pop(coroutine);
        }

        coroutineI_kill_regardless(coroutine);
    }

    gcI_reset_safe(I, 0);
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

void throwI_catchable(morphine_coroutine_t U, size_t callstate) {
    struct callinfo *callinfo = callstackI_info(U);

    callinfo->catch.enable = true;
    callinfo->catch.state = callstate;
    callinfo->catch.space_size = stackI_space(U);
}
