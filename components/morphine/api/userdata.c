//
// Created by whyiskra on 3/22/24.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"

MORPHINE_API void *mapi_push_userdata(
    morphine_coroutine_t U,
    const char *type,
    size_t size,
    morphine_mark_t mark,
    morphine_free_t free
) {
    struct userdata *userdata = userdataI_create(U->I, type, size, mark, free);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_push_userdata_vec(
    morphine_coroutine_t U,
    const char *type,
    size_t count,
    size_t size,
    morphine_mark_t mark,
    morphine_free_t free
) {
    struct userdata *userdata = userdataI_create_vec(U->I, type, count, size, mark, free);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_userdata_resize(morphine_coroutine_t U, size_t size) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    userdataI_resize(U->I, userdata, size);
    return userdata->data;
}

MORPHINE_API void *mapi_userdata_resize_vec(morphine_coroutine_t U, size_t count, size_t size) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    userdataI_resize_vec(U->I, userdata, count, size);
    return userdata->data;
}

MORPHINE_API const char *mapi_userdata_type(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->name;
}

MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->data;
}
