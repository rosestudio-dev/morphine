//
// Created by whyiskra on 3/22/24.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"
#include "morphine/object/coroutine/stack/access.h"

MORPHINE_API void mapi_push_userdata(
    morphine_coroutine_t U,
    const char *type,
    void *pointer,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    struct userdata *userdata = userdataI_create(U->I, type, pointer, mark, free);
    stackI_push(U, valueI_object(userdata));
}

MORPHINE_API const char *mapi_userdata_type(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->type;
}

MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t U) {
    return valueI_as_userdata_or_error(U->I, stackI_peek(U, 0))->data;
}

MORPHINE_API void mapi_userdata_link(morphine_coroutine_t U, bool soft) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 1));
    struct userdata *linking = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));

    userdataI_link(U->I, userdata, linking, soft);
}

MORPHINE_API bool mapi_userdata_unlink(morphine_coroutine_t U, void *pointer) {
    struct userdata *userdata = valueI_as_userdata_or_error(U->I, stackI_peek(U, 0));
    return userdataI_unlink(U->I, userdata, pointer);
}
