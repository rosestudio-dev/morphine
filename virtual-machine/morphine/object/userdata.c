//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/userdata.h"
#include "morphine/core/allocator.h"
#include "morphine/utils/unused.h"

struct userdata *userdataI_create(morphine_instance_t I, void *p, morphine_userdata_mark_t mark, morphine_userdata_free_t free) {
    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata));

    (*result) = (struct userdata) {
        .free = free,
        .mark = mark,
        .metatable = NULL,
        .userdata = p
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    if (userdata->free != NULL) {
        userdata->free(I, userdata->userdata);
    }

    allocI_free(I, userdata);
}

size_t userdataI_allocated_size(struct userdata *userdata) {
    morphinem_unused(userdata);

    return sizeof(struct userdata);
}
