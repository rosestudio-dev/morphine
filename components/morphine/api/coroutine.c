//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

MORPHINE_API morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t U) {
    morphine_coroutine_t coroutine = coroutineI_create(U->I);
    stackI_push(U, valueI_object(coroutine));

    return coroutine;
}

MORPHINE_API void mapi_push_current_coroutine(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U));
}

MORPHINE_API morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t U) {
    return valueI_as_coroutine_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API void mapi_attach(morphine_coroutine_t U) {
    coroutineI_attach(U);
}

MORPHINE_API void mapi_coroutine_suspend(morphine_coroutine_t U) {
    coroutineI_suspend(U);
}

MORPHINE_API void mapi_coroutine_kill(morphine_coroutine_t U) {
    coroutineI_kill(U);
}

MORPHINE_API void mapi_coroutine_resume(morphine_coroutine_t U) {
    coroutineI_resume(U);
}

MORPHINE_API void mapi_coroutine_priority(morphine_coroutine_t U, priority_t priority) {
    coroutineI_priority(U, priority);
}

MORPHINE_API const char *mapi_coroutine_status(morphine_coroutine_t U) {
    return coroutineI_status2string(U, U->status);
}

MORPHINE_API bool mapi_coroutine_isalive(morphine_coroutine_t U) {
    return coroutineI_isalive(U);
}
