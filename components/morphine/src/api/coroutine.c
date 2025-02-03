//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

MORPHINE_API morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t U) {
    struct string *name = valueI_as_string_or_error(U->I, stackI_peek(U, 0));

    morphine_coroutine_t coroutine = coroutineI_create(U->I, name, U->env);
    stackI_replace(U, 0, valueI_object(coroutine));

    return coroutine;
}

MORPHINE_API morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t U) {
    return valueI_as_coroutine_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API void mapi_current(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U));
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

MORPHINE_API const char *mapi_coroutine_status(morphine_coroutine_t U) {
    return coroutineI_status2string(U, U->status);
}

MORPHINE_API bool mapi_coroutine_is_alive(morphine_coroutine_t U) {
    return coroutineI_isalive(U);
}

MORPHINE_API void mapi_coroutine_name(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->name));
}

MORPHINE_API void mapi_push_env(morphine_coroutine_t U) {
    stackI_push(U, U->env);
}

MORPHINE_API void mapi_change_env(morphine_coroutine_t U) {
    U->env = stackI_peek(U, 0);
    stackI_pop(U, 1);
}
