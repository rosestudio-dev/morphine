//
// Created by why-iskra on 30.09.2024.
//

#include "morphine/object/exception.h"
#include "morphine/core/convert.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/stream.h"
#include "morphine/utils/overflow.h"

#define plural_suffix(n) ((n) == 1 ? "" : "s")

struct exception *exceptionI_create(morphine_instance_t I, struct value value, exception_kind_t kind) {
    gcI_safe_enter(I);
    gcI_safe(I, value);

    // create
    struct exception *result = allocI_uni(I, NULL, sizeof(struct exception));
    (*result) = (struct exception) {
        .kind = kind,
        .value = value,
        .stacktrace.recorded = false,
        .stacktrace.printable = false,
        .stacktrace.size = 0,
        .stacktrace.name = NULL,
        .stacktrace.elements = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_EXCEPTION);

    gcI_safe_exit(I);

    return result;
}

void exceptionI_free(morphine_instance_t I, struct exception *exception) {
    allocI_free(I, exception->stacktrace.elements);
    allocI_free(I, exception);
}

const char *exceptionI_kind2str(morphine_instance_t I, exception_kind_t kind) {
    switch (kind) {
        case EXCEPTION_KIND_USER: return "user";
        case EXCEPTION_KIND_OFM: return "ofm";
        case EXCEPTION_KIND_AF: return "af";
        case EXCEPTION_KIND_UNDEF: return "undef";
    }

    throwI_panic(I, "unsupported exception kind");
}

const char *exceptionI_stacktrace_type2str(morphine_instance_t I, stacktrace_type_t type) {
    switch (type) {
        case STACKTRACE_TYPE_NONE: return "none";
        case STACKTRACE_TYPE_FUNCTION: return "function";
        case STACKTRACE_TYPE_NATIVE: return "native";
    }

    throwI_panic(I, "unsupported stacktrace element type");

}

void exceptionI_stacktrace_stub(morphine_instance_t I, struct exception *exception) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    if (exception->stacktrace.recorded) {
        throwI_error(I, "stacktrace already recorded");
    }

    exception->stacktrace.recorded = true;
    exception->stacktrace.printable = false;
    exception->stacktrace.elements = NULL;
    exception->stacktrace.size = 0;
    exception->stacktrace.name = NULL;
}

struct string *exceptionI_message(morphine_instance_t I, struct exception *exception) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    struct value value = exception->value;
    if (!metatableI_test(I, value, MTYPE_METAFIELD_MESSAGE, &value)) {
        value = exception->value;
    }

    gcI_safe_enter(I);

    gcI_safe(I, valueI_object(exception));
    gcI_safe(I, value);
    struct string *string = gcI_safe_obj(I, string, convertI_to_string(I, value));

    gcI_safe_exit(I);

    return string;
}

static void print_string(morphine_instance_t I, struct stream *stream, struct string *string) {
    if (string == NULL) {
        streamI_print(I, stream, "(no string)");
    } else if (string->size > MPARAM_STACKTRACE_LINE_LEN) {
        streamI_write(I, stream, (const uint8_t *) (string->chars), MPARAM_STACKTRACE_LINE_LEN * sizeof(char));
        streamI_print(I, stream, "...");
    } else {
        streamI_write(I, stream, (const uint8_t *) (string->chars), (size_t) string->size * sizeof(char));
    }
}

void exceptionI_error_print(morphine_instance_t I, struct exception *exception, struct stream *stream) {
    gcI_safe_enter(I);

    gcI_safe(I, valueI_object(exception));
    gcI_safe(I, valueI_object(stream));
    struct string *string = gcI_safe_obj(I, string, exceptionI_message(I, exception));

    streamI_print(I, stream, "morphine error: ");
    print_string(I, stream, string);
    streamI_print(I, stream, "\n");

    gcI_safe_exit(I);
}

void exceptionI_stacktrace_print(
    morphine_instance_t I,
    struct exception *exception,
    struct stream *stream,
    ml_size count
) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    if (!exception->stacktrace.recorded) {
        streamI_print(I, stream, "stacktrace wasn't recorded");
        return;
    }

    if (!exception->stacktrace.printable) {
        return;
    }

    ml_size size = exception->stacktrace.size;

    if (exception->stacktrace.name != NULL) {
        streamI_printf(
            I,
            stream,
            "tracing callstack (%" MLIMIT_SIZE_PR " element%s, for coroutine '",
            size,
            plural_suffix(size)
        );
        print_string(I, stream, exception->stacktrace.name);
        streamI_print(I, stream, "'):\n");
    } else {
        streamI_printf(
            I,
            stream,
            "tracing callstack (%" MLIMIT_SIZE_PR " element%s, unnamed coroutine):\n",
            size,
            plural_suffix(size)
        );
    }

    for (ml_size i = 0; i < size; i++) {
        struct stacktrace_element *element = exception->stacktrace.elements + i;

        if (size > count) {
            ml_size count_before = count / 2;
            ml_size count_after = count - count_before;
            ml_size reversed = size - i - 1;

            if (reversed == count_before) {
                ml_size skipped = size - count;
                streamI_printf(
                    I,
                    stream,
                    "    (skipped %" MLIMIT_SIZE_PR " element%s)\n",
                    skipped,
                    plural_suffix(skipped)
                );
            }

            if (reversed >= count_before && reversed < size - count_after) {
                continue;
            }
        }

        streamI_print(I, stream, "    ");
        switch (element->type) {
            case STACKTRACE_TYPE_FUNCTION: {
                if (element->line == 0) {
                    streamI_print(I, stream, "[line: undefined]");
                } else {
                    streamI_printf(I, stream, "[line: %" MLIMIT_LINE_PR "]", element->line);
                }

                streamI_print(I, stream, " function '");
                print_string(I, stream, element->name);
                streamI_printf(I, stream, "'\n");
                break;
            }
            case STACKTRACE_TYPE_NATIVE: {
                streamI_printf(I, stream, "[state: %" MLIMIT_SIZE_PR "] native '", element->state);
                print_string(I, stream, element->name);
                streamI_printf(I, stream, "'\n");
                break;
            }
            default: {
                streamI_printf(I, stream, "(unsupported callstack element)\n");
                break;
            }
        }
    }
}

void exceptionI_stacktrace_record(morphine_instance_t I, struct exception *exception, morphine_coroutine_t coroutine) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    if (exception->stacktrace.recorded) {
        throwI_error(I, "stacktrace already recorded");
    }

    ml_size size = 0;

    {
        struct callframe *frame = coroutine->callstack.frame;
        while (frame != NULL) {
            size = mm_overflow_opc_add(size, 1, throwI_error(I, "stacktrace overflow"));
            frame = frame->prev;
        }
    }

    exception->stacktrace.elements = allocI_vec(I, NULL, size, sizeof(struct stacktrace_element));
    exception->stacktrace.recorded = true;
    exception->stacktrace.printable = true;
    exception->stacktrace.size = size;
    for (ml_size i = 0; i < size; i++) {
        exception->stacktrace.elements[i] = (struct stacktrace_element) {
            .type = STACKTRACE_TYPE_NONE,
            .name = NULL,
            .line = 0,
            .state = 0,
        };
    }

    exception->stacktrace.name = gcI_objbarrier(I, exception, coroutine->name);

    struct callframe *frame = coroutine->callstack.frame;
    for (ml_size i = 0; i < size; i++) {
        struct stacktrace_element *element = exception->stacktrace.elements + i;
        if (frame == NULL) {
            break;
        } else {
            struct value callable = callstackI_extract_callable(I, *frame->s.direct.callable);
            if (valueI_is_function(callable)) {
                ml_line line = 0;
                struct function *function = valueI_as_function(callable);
                if (function->instructions_count > 0) {
                    if (frame->pc.position < function->instructions_count) {
                        line = function->instructions[frame->pc.position].line;
                    } else {
                        line = function->instructions[function->instructions_count - 1].line;
                    }
                }

                (*element) = (struct stacktrace_element) {
                    .type = STACKTRACE_TYPE_FUNCTION,
                    .name = function->name,
                    .line = line,
                    .state = 0,
                };
            } else if (valueI_is_native(callable)) {
                struct native *native = valueI_as_native(callable);

                (*element) = (struct stacktrace_element) {
                    .type = STACKTRACE_TYPE_NATIVE,
                    .name = native->name,
                    .line = 0,
                    .state = frame->pc.state,
                };
            }

            gcI_objbarrier(I, exception, element->name);

            frame = frame->prev;
        }
    }
}

struct stacktrace_element exceptionI_stacktrace_element(
    morphine_instance_t I,
    struct exception *exception,
    ml_size index
) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    if (!exception->stacktrace.recorded) {
        throwI_error(I, "stacktrace wasn't recorded");
    }

    if (index >= exception->stacktrace.size) {
        throwI_error(I, "stacktrace index is out of bounce");
    }

    return exception->stacktrace.elements[index];
}
