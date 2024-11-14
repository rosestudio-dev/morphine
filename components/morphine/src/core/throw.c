//
// Created by whyiskra on 16.12.23.
//

#include <setjmp.h>
#include <memory.h>
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/string.h"
#include "morphine/object/exception.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "morphine/object/sio.h"

#define OFM_MESSAGE ("out of memory")

morphine_noret static void panic(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    throw->protect.entered = false;
    throw->signal_entered++;
    I->platform.functions.signal(I);
}

morphine_noret static void error(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    if (throw->protect.entered) {
        longjmp(throw->protect.handler, 1);
    }

    panic(I);
}

struct throw throwI_prototype(void) {
    return (struct throw) {
        .context = NULL,
        .signal_entered = 0,
        .protect.entered = false,
        .type = THROW_TYPE_VALUE,
        .error.value = valueI_nil,
        .special.ofm = NULL,
    };
}

void throwI_special(morphine_instance_t I) {
    struct string *text = stringI_create(I, OFM_MESSAGE);
    struct exception *exception = exceptionI_create(I, valueI_object(text));
    exceptionI_stacktrace_stub(I, exception);

    I->throw.special.ofm = exception;
}

void throwI_handler(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    morphine_coroutine_t coroutine = throw->context;

    if (coroutine == NULL) {
        panic(I);
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

    if (callinfo != NULL && callinfo->catch.crash) {
        panic(I);
    }

    stackI_throw_fix(coroutine);
    callstackI_throw_fix(coroutine);

    struct exception *exception;
    switch (throw->type) {
        case THROW_TYPE_VALUE: {
            exception = exceptionI_create(I, throw->error.value);
            gcI_safe_obj(I, objectI_cast(exception));
            exceptionI_stacktrace_record(I, exception, coroutine);
            break;
        }
        case THROW_TYPE_MESSAGE: {
            struct value value = valueI_object(stringI_create(I, throw->error.message));
            gcI_safe_value(I, value);
            exception = exceptionI_create(I, value);
            gcI_safe_obj(I, objectI_cast(exception));
            exceptionI_stacktrace_record(I, exception, coroutine);
            break;
        }
        case THROW_TYPE_OFM: {
            if (I->throw.special.ofm == NULL) {
                struct value value = valueI_object(stringI_create(I, OFM_MESSAGE));
                gcI_safe_value(I, value);
                exception = exceptionI_create(I, value);
                gcI_safe_obj(I, objectI_cast(exception));
                exceptionI_stacktrace_stub(I, exception);
            } else {
                exception = I->throw.special.ofm;
            }
            break;
        }
        default: {
            throwI_panic(I, "unsupported throw type");
        }
    }

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
        exceptionI_stacktrace_print(I, exception, I->sio.error, MPARAM_TRACESTACK_COUNT);

        while (callstackI_info(coroutine) != NULL) {
            callstackI_pop(coroutine);
        }

        coroutineI_kill(coroutine);
    }

    throw->context = NULL;
    gcI_reset_safe(I, 0);
}

morphine_noret void throwI_error(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_MESSAGE;
    throw->error.message = message;
    error(I);
}

morphine_noret void throwI_errorv(morphine_instance_t I, struct value value) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_VALUE;
    throw->error.value = value;
    error(I);
}

morphine_noret void throwI_panic(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_MESSAGE;
    throw->error.message = message;
    panic(I);
}

morphine_noret void throwI_ofm(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_OFM;
    error(I);
}

void throwI_protect(
    morphine_instance_t I,
    morphine_try_t try,
    morphine_catch_t catch,
    void *try_data,
    void *catch_data
) {
    struct throw *throw = &I->throw;

    struct protect_frame previous;
    memcpy(&previous, &throw->protect, sizeof(struct protect_frame));

    if (setjmp(throw->protect.handler) != 0) {
        memcpy(&throw->protect, &previous, sizeof(struct protect_frame));
        catch(catch_data);
    } else {
        throw->protect.entered = true;
        try(try_data);

        memcpy(&throw->protect, &previous, sizeof(struct protect_frame));
    }
}

const char *throwI_message(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    switch (throw->type) {
        case THROW_TYPE_VALUE: {
            struct string *string = valueI_safe_as_string(throw->error.value, NULL);
            throw->error.value = valueI_nil;

            if (string != NULL && stringI_is_cstr_compatible(I, string)) {
                return string->chars;
            }

            return "(unsupported value)";
        }
        case THROW_TYPE_MESSAGE: {
            return throw->error.message;
        }
        case THROW_TYPE_OFM: {
            return OFM_MESSAGE;
        }
    }

    return "unsupported throw type";
}

bool throwI_is_nested_signal(morphine_instance_t I) {
    return I->throw.signal_entered > 1;
}

void throwI_catchable(morphine_coroutine_t U, size_t callstate) {
    struct callinfo *callinfo = callstackI_info(U);

    callinfo->catch.enable = true;
    callinfo->catch.crash = false;
    callinfo->catch.state = callstate;
}

void throwI_crashable(morphine_coroutine_t U) {
    struct callinfo *callinfo = callstackI_info(U);

    callinfo->catch.enable = true;
    callinfo->catch.crash = true;
    callinfo->catch.state = 0;
}

void throwI_uncatch(morphine_coroutine_t U) {
    struct callinfo *callinfo = callstackI_info(U);

    callinfo->catch.enable = false;
    callinfo->catch.crash = false;
    callinfo->catch.state = 0;
}

struct value throwI_thrown(morphine_coroutine_t U) {
    struct value result = U->thrown;
    U->thrown = valueI_nil;
    return result;
}
