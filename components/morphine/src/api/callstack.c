//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_call(morphine_coroutine_t U, ml_size argc) {
    callstackI_call_api(U, argc);
}

MORPHINE_API void mapi_push_callable(morphine_coroutine_t U) {
    stackI_push(U, callstackI_callable(U));
}

MORPHINE_API void mapi_extract_source(morphine_coroutine_t U) {
    stackI_push(U, callstackI_extract_callable(U->I, stackI_peek(U, 0)));
}

MORPHINE_API void mapi_push_result(morphine_coroutine_t U) {
    stackI_push(U, callstackI_result(U));
}

MORPHINE_API void mapi_set_result(morphine_coroutine_t U) {
    callstackI_set_result(U, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_return(morphine_coroutine_t U) {
    callstackI_pop(U, stackI_peek(U, 0));
}

MORPHINE_API void mapi_leave(morphine_coroutine_t U) {
    callstackI_pop(U, valueI_nil);
}

MORPHINE_API void mapi_continue(morphine_coroutine_t U, ml_size callstate) {
    callstackI_continue(U, callstate);
}

MORPHINE_API ml_size mapi_callstate(morphine_coroutine_t U) {
    return callstackI_state(U);
}

MORPHINE_API ml_size mapi_args(morphine_coroutine_t U) {
    return callstackI_args(U);
}

MORPHINE_API void mapi_push_arg(morphine_coroutine_t U, ml_size index) {
    stackI_push(U, callstackI_get_arg(U, index));
}
