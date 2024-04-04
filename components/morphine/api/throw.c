//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"
#include "morphine/stack/access.h"
#include "morphine/stack/call.h"

MORPHINE_API morphine_noret void mapi_errorf(morphine_state_t S, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(S->I, str, args);
    va_end(args);

    throwI_errorv(S->I, valueI_object(result));
}

MORPHINE_API morphine_noret void mapi_error(morphine_state_t S) {
    throwI_errorv(S->I, stackI_peek(S, 0));
}

MORPHINE_API morphine_noret void mapi_panic(morphine_state_t S, const char *message) {
    if (message == NULL) {
        struct value value = stackI_peek(S, 0);
        throwI_panicv(S->I, value);
    } else {
        throwI_panic(S->I, message);
    }
}

MORPHINE_API void mapi_catchable(morphine_state_t S, size_t callstate) {
    throwI_catchable(S, callstate);
}

MORPHINE_API void mapi_push_thrown(morphine_state_t S) {
    stackI_push(S, *callstackI_info_or_error(S)->s.thrown.p);
}

MORPHINE_API const char *mapi_get_panic_message(morphine_instance_t I) {
    if (I == NULL) {
        return "Initial error";
    }

    return throwI_message(I);
}
