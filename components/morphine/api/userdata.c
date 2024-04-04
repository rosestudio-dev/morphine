//
// Created by whyiskra on 3/22/24.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/state.h"
#include "morphine/object/userdata.h"
#include "morphine/stack/access.h"

MORPHINE_API void mapi_push_userdata(
    morphine_state_t S,
    const char *type,
    void *pointer,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    struct userdata *userdata = userdataI_create(S->I, type, pointer, mark, free);
    stackI_push(S, valueI_object(userdata));
}

MORPHINE_API const char *mapi_userdata_type(morphine_state_t S) {
    return valueI_as_userdata_or_error(S->I, stackI_peek(S, 0))->type;
}

MORPHINE_API void *mapi_userdata_pointer(morphine_state_t S) {
    return valueI_as_userdata_or_error(S->I, stackI_peek(S, 0))->data;
}

MORPHINE_API void mapi_userdata_link(morphine_state_t S, bool soft) {
    struct userdata *userdata = valueI_as_userdata_or_error(S->I, stackI_peek(S, 1));
    struct userdata *linking = valueI_as_userdata_or_error(S->I, stackI_peek(S, 0));

    userdataI_link(S->I, userdata, linking, soft);
}

MORPHINE_API bool mapi_userdata_unlink(morphine_state_t S, void *pointer) {
    struct userdata *userdata = valueI_as_userdata_or_error(S->I, stackI_peek(S, 0));
    return userdataI_unlink(S->I, userdata, pointer);
}
