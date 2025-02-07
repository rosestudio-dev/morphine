//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/exception.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/stream.h"
#include "morphine/object/string.h"
#include <memory.h>
#include <setjmp.h>

#define SIGNAL_RECURSION (1024)
#define DANGER_RECURSION (1024)

#define OFM_MESSAGE   ("out of memory")
#define AF_MESSAGE    ("allocation fault")
#define UNDEF_MESSAGE ("undefined thrown")

#define SPECIAL(t) t##_MESSAGE, EXCEPTION_KIND_##t

static inline void fix(morphine_instance_t I) {
#define fix_coroutine(I, t) do { morphine_coroutine_t coroutine = (I)->throw.fix.t; if (coroutine != NULL) { throwI_danger_enter((I)); coroutineI_fix_##t(coroutine); throwI_danger_exit((I)); } (I)->throw.fix.t = NULL; } while (0)
    fix_coroutine(I, stack);
    fix_coroutine(I, callstack);
#undef fix_coroutine
}

mattr_noret static void csignal(morphine_instance_t I, bool is_panic) {
    struct throw *throw = &I->throw;

    throw->protect.entered = false;

    if (throw->signal_entered >= SIGNAL_RECURSION) {
        throw->type = THROW_TYPE_MESSAGE;
        throw->error.message = "signal recursion detected";
    } else {
        throw->signal_entered++;
    }

    I->platform.signal(I, I->data, is_panic);
}

mattr_noret static void error(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    if (throw->protect.danger_entered > 0) {
        csignal(I, true);
    }

    {
        fix(I);

        throwI_danger_enter(I);
        gcI_safe_reset(I, throw->protect.safe_level);
        throwI_danger_exit(I);
    }

    if (throw->protect.entered) {
        longjmp(throw->protect.handler, 1);
    }

    csignal(I, false);
}

struct throw throwI_prototype(void) {
    return (struct throw) {
        .signal_entered = 0,
        .protect.entered = false,
        .protect.danger_entered = 0,
        .protect.safe_level = 0,
        .type = THROW_TYPE_UNDEF,
        .error.value = valueI_nil,
        .special.ofm = NULL,
        .special.af = NULL,
        .fix.stack = NULL,
        .fix.callstack = NULL,
    };
}

void throwI_destruct(morphine_instance_t I) {
    fix(I);
}

static struct exception *create_special(morphine_instance_t I, const char *msg, exception_kind_t kind) {
    gcI_safe_enter(I);
    struct value text = gcI_safe(I, valueI_object(stringI_create(I, msg)));
    struct exception *exception = gcI_safe_obj(I, exception, exceptionI_create(I, text, kind));
    exceptionI_stacktrace_stub(I, exception);
    gcI_safe_exit(I);

    return exception;
}

void throwI_special(morphine_instance_t I) {
    I->throw.special.ofm = create_special(I, SPECIAL(OFM));
    I->throw.special.af = create_special(I, SPECIAL(AF));
}

mtype_catch_t throwI_interpreter_handler(morphine_instance_t I) {
    struct throw *throw = &I->throw;
    morphine_coroutine_t coroutine = I->interpreter.context;

    if (coroutine == NULL) {
        return MTYPE_CATCH_PROVIDE;
    }

    struct callframe *frame = coroutine->callstack.frame;
    {
        while (frame != NULL) {
            if (frame->params.catch_enabled) {
                if (frame->params.catch_crash) {
                    return MTYPE_CATCH_CRASH;
                }

                break;
            }

            frame = frame->prev;
        }
    }

    gcI_safe_enter(I);
    struct exception *exception = NULL;
    switch (throw->type) {
        case THROW_TYPE_UNDEF: {
            struct value value = gcI_safe(I, valueI_object(stringI_create(I, UNDEF_MESSAGE)));
            exception = gcI_safe_obj(I, exception, exceptionI_create(I, value, EXCEPTION_KIND_UNDEF));
            exceptionI_stacktrace_record(I, exception, coroutine);
            break;
        }
        case THROW_TYPE_VALUE: {
            exception = gcI_safe_obj(I, exception, exceptionI_create(I, throw->error.value, EXCEPTION_KIND_USER));
            exceptionI_stacktrace_record(I, exception, coroutine);
            break;
        }
        case THROW_TYPE_MESSAGE: {
            struct value value = gcI_safe(I, valueI_object(stringI_create(I, throw->error.message)));
            exception = gcI_safe_obj(I, exception, exceptionI_create(I, value, EXCEPTION_KIND_USER));
            exceptionI_stacktrace_record(I, exception, coroutine);
            break;
        }
        case THROW_TYPE_OFM: {
            if (I->throw.special.ofm == NULL) {
                exception = create_special(I, SPECIAL(OFM));
            } else {
                exception = I->throw.special.ofm;
            }
            break;
        }
        case THROW_TYPE_AF: {
            if (I->throw.special.af == NULL) {
                exception = create_special(I, SPECIAL(AF));
            } else {
                exception = I->throw.special.af;
            }
            break;
        }
    }

    if (exception == NULL) {
        throwI_panic(I, "unsupported throw type");
    }

    if (frame != NULL) {
        // set state
        frame->pc.state = frame->params.catch_state;
        frame->params.catch_enabled = false;

        // pop while catch
        callstackI_throw_drop(coroutine, frame);
    } else {
        exceptionI_error_print(I, exception, I->stream.err);
        exceptionI_stacktrace_print(I, exception, I->stream.err, MPARAM_STACKTRACE_COUNT);
        coroutineI_kill(coroutine);
    }

    // set error value
    coroutine->exception = exception;

    I->interpreter.context = NULL;
    gcI_safe_exit(I);

    return MTYPE_CATCH_SUCCESS;
}

mattr_noret void throwI_error(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_MESSAGE;
    throw->error.message = message;
    error(I);
}

mattr_noret void throwI_errorv(morphine_instance_t I, struct value value) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_VALUE;
    throw->error.value = value;
    error(I);
}

mattr_noret void throwI_errorf(morphine_instance_t I, const char *message, ...) {
    va_list args;
    va_start(args, message);
    struct string *error = stringI_createva(I, message, args);
    va_end(args);

    throwI_errorv(I, valueI_object(error));
}

mattr_noret void throwI_panic(morphine_instance_t I, const char *message) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_MESSAGE;
    throw->error.message = message;
    csignal(I, true);
}

mattr_noret void throwI_panicv(morphine_instance_t I, struct value value) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_VALUE;
    throw->error.value = value;
    csignal(I, true);
}

mattr_noret void throwI_panicf(morphine_instance_t I, const char *message, ...) {
    va_list args;
    va_start(args, message);
    struct string *error = stringI_createva(I, message, args);
    va_end(args);

    throwI_panicv(I, valueI_object(error));
}

mattr_noret void throwI_ofm(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_OFM;
    error(I);
}

mattr_noret void throwI_af(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_AF;
    error(I);
}

mattr_noret void throwI_undef(morphine_instance_t I) {
    struct throw *throw = &I->throw;

    throw->type = THROW_TYPE_UNDEF;
    error(I);
}

mattr_noret void throwI_provide_error(morphine_coroutine_t U) {
    struct exception *exception = coroutineI_exception(U);
    if(exception != NULL) {
        switch (exception->kind) {
            case EXCEPTION_KIND_USER: throwI_errorv(U->I, exception->value);
            case EXCEPTION_KIND_OFM: throwI_ofm(U->I);
            case EXCEPTION_KIND_AF: throwI_af(U->I);
            case EXCEPTION_KIND_UNDEF: throwI_undef(U->I);
        }
    }

    throwI_undef(U->I);
}

void throwI_danger_enter(morphine_instance_t I) {
    if (I->throw.protect.danger_entered >= DANGER_RECURSION) {
        throwI_panic(I, "danger section recursion detected");
    } else {
        I->throw.protect.danger_entered++;
    }
}

void throwI_danger_exit(morphine_instance_t I) {
    if (I->throw.protect.danger_entered == 0) {
        throwI_panic(I, "danger section corrupted");
    } else {
        I->throw.protect.danger_entered--;
    }
}

void throwI_protect(
    morphine_instance_t I,
    mfunc_try_t try,
    mfunc_catch_t catch,
    void *try_data,
    void *catch_data
) {
    struct throw *throw = &I->throw;

    struct protect_frame previous;
    memcpy(&previous, &throw->protect, sizeof(struct protect_frame));

    if (setjmp(throw->protect.handler) != 0) {
        memcpy(&throw->protect, &previous, sizeof(struct protect_frame));
        mtype_catch_t result = catch(catch_data);

        switch (result) {
            case MTYPE_CATCH_SUCCESS: return;
            case MTYPE_CATCH_PROVIDE: error(I);
            case MTYPE_CATCH_CRASH: csignal(I, false);
        }

        throwI_panic(I, "unsupported catch result");
    } else {
        throw->protect.safe_level = gcI_safe_level(I);
        throw->protect.danger_entered = 0;
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

            if (string != NULL && stringI_is_cstr(string)) {
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
        case THROW_TYPE_AF: {
            return AF_MESSAGE;
        }
        case THROW_TYPE_UNDEF: {
            return UNDEF_MESSAGE;
        }
    }

    return "unsupported throw type";
}

bool throwI_is_nested_signal(morphine_instance_t I) {
    return I->throw.signal_entered > 1;
}
