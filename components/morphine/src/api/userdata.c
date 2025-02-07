//
// Created by whyiskra on 3/22/24.
//

#include "morphine/object/userdata.h"
#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/core/usertype.h"
#include "morphine/object/coroutine.h"
#include <string.h>

MORPHINE_API void *mapi_push_userdata(morphine_coroutine_t U, const char *type) {
    struct userdata *userdata = userdataI_instance(U->I, type);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_push_userdata_uni(
    morphine_coroutine_t U,
    size_t size,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor
) {
    struct userdata *userdata = userdataI_create_uni(U->I, size, constructor, destructor);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_push_userdata_vec(
    morphine_coroutine_t U,
    size_t count,
    size_t size,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor
) {
    struct userdata *userdata = userdataI_create_vec(U->I, count, size, constructor, destructor);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t U, const char *type) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    return userdataI_pointer(U->I, userdata, type);
}

MORPHINE_API bool mapi_userdata_is_typed(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->is_typed;
}
