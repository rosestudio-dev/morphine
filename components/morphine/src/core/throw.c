//
// Created by whyiskra on 16.12.23.
//

#include <setjmp.h>
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/string.h"
#include "morphine/object/exception.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "morphine/object/sio.h"

struct throw throwI_prototype(void) {
    return (struct throw) {
        .inited = false,
        .signal_entered = 0,
        .is_message = false,
        .error.value = valueI_nil,
        .context = NULL
    };
}

void throwI_handler(morphine_instance_t I) {
    struct throw *throw = &I->E.throw;
    I->E.throw.inited = false;

    morphine_coroutine_t coroutine = throw->context;

    if (coroutine == NULL) {
        I->platform.functions.signal(I);
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

    stackI_throw_fix(coroutine);
    callstackI_throw_fix(coroutine);

    struct value value;
    if (throw->is_message) {
        value = valueI_object(stringI_create(I, throw->error.message));
    } else {
        value = throw->error.value;
    }

    struct exception *exception = exceptionI_create(I, value);
    gcI_safe_obj(I, objectI_cast(exception));
    exceptionI_stacktrace_record(I, exception, coroutine);

    if (callinfo != NULL) {

        // set state
        callinfo->pc.state = callinfo->catch.state;
        callinfo->catch.enable = false;

        // pop while catch
        while (callstackI_info(coroutine) != callinfo) {
            callstackI_pop(coroutine);
        }

        // set error value
        coroutine->thrown = valueI_object(exception);

        size_t stack_size = stackI_space(coroutine);
        stackI_pop(coroutine, stack_size);
    } else {
        exceptionI_error_print(I, exception, I->sio.error);
        exceptionI_stacktrace_print(I, exception, I->sio.error);

        while (callstackI_info(coroutine) != NULL) {
            callstackI_pop(coroutine);
        }

        coroutineI_kill(coroutine);
    }

    throw->context = NULL;
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

    throw->signal_entered++;
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

    throw->signal_entered++;
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
    throw->error.value = valueI_nil;

    if (string != NULL && stringI_is_cstr_compatible(I, string)) {
        return string->chars;
    } else {
        return "(unsupported value)";
    }
}

bool throwI_is_nested_signal(morphine_instance_t I) {
    return I->E.throw.signal_entered > 1;
}

void throwI_catchable(morphine_coroutine_t U, size_t callstate) {
    struct callinfo *callinfo = callstackI_info(U);

    callinfo->catch.enable = true;
    callinfo->catch.state = callstate;
}
