//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/function.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc/loader.h"

MORPHINE_API void mapi_push_function(morphine_coroutine_t U) {
    struct sio *sio = valueI_as_sio_or_error(U->I, stackI_peek(U, 0));
    struct function *result = loaderI_load(U, sio);
    stackI_push(U, valueI_object(result));
}

MORPHINE_API void mapi_static_get(morphine_coroutine_t U, size_t index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    struct value value = functionI_static_get(U->I, function, index);
    stackI_push(U, value);
}

MORPHINE_API void mapi_static_set(morphine_coroutine_t U, size_t index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 1));
    functionI_static_set(U->I, function, index, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API size_t mapi_static_size(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 1));
    return function->statics_count;
}

MORPHINE_API void mapi_constant_get(morphine_coroutine_t U, size_t index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    struct value value = functionI_constant_get(U->I, function, index);
    stackI_push(U, value);
}

MORPHINE_API void mapi_constant_set(morphine_coroutine_t U, size_t index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 1));
    functionI_constant_set(U->I, function, index, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API size_t mapi_constant_size(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 1));
    return function->constants_count;
}
