//
// Created by why-iskra on 30.09.2024.
//

#include "morphine/auxiliary/native.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_push_native(morphine_coroutine_t U, const char *name, mfunc_native_t native) {
    mapi_push_string(U, name);
    mapi_push_native(U, native);
}
