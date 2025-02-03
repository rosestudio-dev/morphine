//
// Created by why-iskra on 30.09.2024.
//

#include "morphine/api.h"
#include "morphine/object/exception.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/object/string.h"

MORPHINE_API void mapi_push_exception(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct exception *exception = exceptionI_create(U->I, value, EXCEPTION_KIND_USER);
    stackI_push(U, valueI_object(exception));
}

MORPHINE_API const char *mapi_exception_kind(morphine_coroutine_t U) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    return exceptionI_kind2str(U->I, exception->kind);
}

MORPHINE_API void mapi_exception_value(morphine_coroutine_t U) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, exception->value);
}

MORPHINE_API void mapi_exception_message(morphine_coroutine_t U) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    struct string *message = exceptionI_message(U->I, exception);
    stackI_push(U, valueI_object(message));
}

MORPHINE_API void mapi_exception_error_print(morphine_coroutine_t U) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 1));
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    exceptionI_error_print(U->I, exception, stream);
}

MORPHINE_API void mapi_exception_stacktrace_print(morphine_coroutine_t U, ml_size count) {
    struct stream *stream = valueI_as_stream_or_error(U->I, stackI_peek(U, 1));
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    exceptionI_stacktrace_print(U->I, exception, stream, count);
}

MORPHINE_API void mapi_exception_stacktrace_record(morphine_coroutine_t U, morphine_coroutine_t coroutine) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    exceptionI_stacktrace_record(U->I, exception, coroutine);
}

MORPHINE_API void mapi_exception_stacktrace_name(morphine_coroutine_t U) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    if (exception->stacktrace.name == NULL) {
        stackI_push(U, valueI_nil);
    } else {
        stackI_push(U, exception->value);
    }
}

MORPHINE_API ml_size mapi_exception_stacktrace_size(morphine_coroutine_t U) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    return exception->stacktrace.size;
}

MORPHINE_API const char *mapi_exception_stacktrace_type(morphine_coroutine_t U, ml_size index) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    return exceptionI_stacktrace_element(U->I, exception, index).type;
}

MORPHINE_API void mapi_exception_stacktrace_callable(morphine_coroutine_t U, ml_size index) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    struct stacktrace_parsed_element element = exceptionI_stacktrace_element(U->I, exception, index);

    if (element.name != NULL) {
        stackI_push(U, valueI_object(element.name));
    } else {
        stackI_push(U, valueI_nil);
    }
}

MORPHINE_API ml_line mapi_exception_stacktrace_line(morphine_coroutine_t U, ml_size index) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    return exceptionI_stacktrace_element(U->I, exception, index).line;

}

MORPHINE_API ml_size mapi_exception_stacktrace_state(morphine_coroutine_t U, ml_size index) {
    struct exception *exception = valueI_as_exception_or_error(U->I, stackI_peek(U, 0));
    return exceptionI_stacktrace_element(U->I, exception, index).state;
}
