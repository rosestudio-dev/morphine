//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"
#include "morphine/object/coroutine/stack/access.h"
#include "morphine/object/coroutine/stack/call.h"

MORPHINE_API morphine_noret void mapi_errorf(morphine_coroutine_t U, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(U->I, str, args);
    va_end(args);

    throwI_errorv(U->I, valueI_object(result));
}

MORPHINE_API morphine_noret void mapi_error(morphine_coroutine_t U) {
    throwI_errorv(U->I, stackI_peek(U, 0));
}

MORPHINE_API morphine_noret void mapi_panic(morphine_coroutine_t U, const char *message) {
    if (message == NULL) {
        struct value value = stackI_peek(U, 0);
        throwI_panicv(U->I, value);
    } else {
        throwI_panic(U->I, message);
    }
}

MORPHINE_API void mapi_catchable(morphine_coroutine_t U, size_t callstate) {
    throwI_catchable(U, callstate);
}

MORPHINE_API void mapi_push_thrown(morphine_coroutine_t U) {
    stackI_push(U, *callstackI_info_or_error(U)->s.thrown.p);
}

MORPHINE_API const char *mapi_get_panic_message(morphine_instance_t I) {
    if (I == NULL) {
        return "Initial error";
    }

    return throwI_message(I);
}
