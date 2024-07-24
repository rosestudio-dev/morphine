//
// Created by whyiskra on 3/22/24.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"
#include "morphine/core/usertype.h"

MORPHINE_API void *mapi_push_userdata(morphine_coroutine_t U, const char *type) {
    struct usertype *usertype = usertypeI_get(U->I, type);
    struct usertype_info info = usertypeI_info(U->I, usertype);

    struct table *table;
    if (info.require_metatable) {
        table = valueI_as_table_or_error(U->I, stackI_peek(U, 0));
    } else {
        table = NULL;
    }

    struct userdata *userdata = userdataI_instance(U->I, type, table);

    if (info.require_metatable) {
        stackI_replace(U, 0, valueI_object(userdata));
    } else {
        stackI_push(U, valueI_object(userdata));
    }

    return userdata->data;
}

MORPHINE_API void *mapi_push_userdata_uni(morphine_coroutine_t U, size_t size) {
    struct userdata *userdata = userdataI_create(U->I, size);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void *mapi_push_userdata_vec(morphine_coroutine_t U, size_t count, size_t size) {
    struct userdata *userdata = userdataI_create_vec(U->I, count, size);
    stackI_push(U, valueI_object(userdata));
    return userdata->data;
}

MORPHINE_API void mapi_userdata_set_free(morphine_coroutine_t U, morphine_free_t free) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    userdataI_set_free(U->I, userdata, free);
}

MORPHINE_API void mapi_userdata_mode_lock_metatable(morphine_coroutine_t U) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    userdataI_mode_lock_metatable(U->I, userdata);
}

MORPHINE_API void mapi_userdata_mode_lock_size(morphine_coroutine_t U) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    userdataI_mode_lock_size(U->I, userdata);
}

MORPHINE_API bool mapi_userdata_mode_metatable_is_locked(morphine_coroutine_t U) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    return userdata->mode.metatable_locked;
}

MORPHINE_API bool mapi_userdata_mode_size_is_locked(morphine_coroutine_t U) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    return userdata->mode.size_locked;
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

MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->data;
}

MORPHINE_API bool mapi_userdata_is_untyped(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->is_untyped;
}
