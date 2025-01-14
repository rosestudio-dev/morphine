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

struct exception *exceptionI_create(morphine_instance_t I, struct value value) {
    gcI_safe_enter(I);
    gcI_safe(I, value);

    // create
    struct exception *result = allocI_uni(I, NULL, sizeof(struct exception));
    (*result) = (struct exception) {
        .value = value,
        .stacktrace.recorded = false,
        .stacktrace.printable = false,
        .stacktrace.size = 0,
        .stacktrace.name = NULL,
        .stacktrace.elements = NULL
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_EXCEPTION);

    gcI_safe_exit(I);

    return result;
}

void exceptionI_free(morphine_instance_t I, struct exception *exception) {
    allocI_free(I, exception->stacktrace.elements);
    allocI_free(I, exception);
}

struct string *exceptionI_message(morphine_instance_t I, struct exception *exception) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    struct value value = exception->value;
    if (!metatableI_builtin_test(I, value, MORPHINE_METAFIELD_MESSAGE, &value)) {
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
    if (string->size > MLIMIT_STACKTRACE_STRING) {
        streamI_write(I, stream, (const uint8_t *) (string->chars), MLIMIT_STACKTRACE_STRING * sizeof(char));
        streamI_print(I, stream, "...");
    } else {
        streamI_write(I, stream, (const uint8_t *) (string->chars), string->size * sizeof(char));
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
            I, stream, "tracing callstack (%"MLIMIT_SIZE_PR" element%s, for coroutine '",
            size, plural_suffix(size)
        );
        print_string(I, stream, exception->stacktrace.name);
        streamI_print(I, stream, "'):\n");
    } else {
        streamI_printf(
            I, stream, "tracing callstack (%"MLIMIT_SIZE_PR" element%s, unnamed coroutine):\n",
            size, plural_suffix(size)
        );
    }

    for (ml_size i = 0; i < size; i++) {
        struct stacktrace_element element = exception->stacktrace.elements[i];

        if (size > count) {
            ml_size count_before = count / 2;
            ml_size count_after = count - count_before;
            ml_size reversed = size - i - 1;

            if (reversed == count_before) {
                ml_size skipped = size - count;
                streamI_printf(
                    I, stream, "    (skipped %"MLIMIT_SIZE_PR" element%s)\n",
                    skipped, plural_suffix(skipped)
                );
            }

            if (reversed >= count_before && reversed < size - count_after) {
                continue;
            }
        }

        streamI_print(I, stream, "    ");
        if (valueI_is_function(element.callable)) {
            struct function *function = valueI_as_function(element.callable);

            if (function->instructions_count > 0) {
                ml_line line;
                if (element.pc.position < function->instructions_count) {
                    line = function->instructions[element.pc.position].line;
                } else {
                    line = function->instructions[function->instructions_count - 1].line;
                }

                streamI_printf(I, stream, "[line: %"MLIMIT_LINE_PR"]", line);
            } else {
                streamI_print(I, stream, "[line: undefined]");
            }

            streamI_print(I, stream, " function '");
            print_string(I, stream, function->name);
            streamI_printf(I, stream, "' (declared in %"MLIMIT_LINE_PR" line)\n", function->line);
        } else if (valueI_is_native(element.callable)) {
            struct native *native = valueI_as_native(element.callable);

            streamI_printf(I, stream, "[state: %zu] native '", element.pc.state);
            print_string(I, stream, native->name);
            streamI_printf(I, stream, "'\n");
        } else {
            streamI_printf(I, stream, "(unsupported callstack element)\n");
        }
    }
}

struct stacktrace_parsed_element exceptionI_stacktrace_element(
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

    struct stacktrace_element element = exception->stacktrace.elements[index];
    struct stacktrace_parsed_element result = {
        .type = valueI_type(I, element.callable, false),
        .name = NULL,
        .line = 0,
        .state = 0
    };

    if (valueI_is_function(element.callable)) {
        struct function *function = valueI_as_function(element.callable);
        result.name = function->name;

        if (function->instructions_count > 0) {
            if (element.pc.position < function->instructions_count) {
                result.line = function->instructions[element.pc.position].line;
            } else {
                result.line = function->instructions[function->instructions_count - 1].line;
            }
        }
    } else if (valueI_is_native(element.callable)) {
        struct native *native = valueI_as_native(element.callable);
        result.name = native->name;
        result.state = element.pc.state;
    }

    return result;
}

void exceptionI_stacktrace_record(
    morphine_instance_t I,
    struct exception *exception,
    morphine_coroutine_t coroutine
) {
    if (exception == NULL) {
        throwI_error(I, "exception is null");
    }

    if (exception->stacktrace.recorded) {
        throwI_error(I, "stacktrace already recorded");
    }

    ml_size size = 0;

    {
        struct callinfo *callinfo = coroutine->callstack.callinfo;
        while (callinfo != NULL) {
            overflow_add(size, 1, MLIMIT_SIZE_MAX) {
                throwI_error(I, "stacktrace overflow");
            }

            size++;
            callinfo = callinfo->prev;
        }
    }

    exception->stacktrace.elements = allocI_vec(I, NULL, size, sizeof(struct stacktrace_element));
    exception->stacktrace.recorded = true;
    exception->stacktrace.printable = true;
    exception->stacktrace.size = size;
    for (ml_size i = 0; i < size; i++) {
        exception->stacktrace.elements[i] = (struct stacktrace_element) {
            .callable = valueI_nil,
            .pc.position = 0,
            .pc.state = 0
        };
    }

    exception->stacktrace.name = coroutine->name;
    gcI_objbarrier(I, exception, coroutine->name);

    struct callinfo *callinfo = coroutine->callstack.callinfo;
    for (ml_size i = 0; i < size; i++) {
        struct stacktrace_element *element = exception->stacktrace.elements + i;
        if (callinfo == NULL) {
            break;
        } else {
            *element = (struct stacktrace_element) {
                .callable = *callinfo->s.stack.source,
                .pc.position = callinfo->pc.position,
                .pc.state = callinfo->pc.state
            };
            gcI_barrier(I, exception, element->callable);

            callinfo = callinfo->prev;
        }
    }
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
