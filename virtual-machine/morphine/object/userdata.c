//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"

struct userdata *userdataI_create(
    morphine_instance_t I,
    const char *type,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    if (type == NULL) {
        throwI_message_panic(I, NULL, "Userdata type is null");
    }

    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata));

    size_t type_len = strlen(type) + 1;

    (*result) = (struct userdata) {
        .type = allocI_uni(I, NULL, type_len),
        .free = free,
        .mark = mark,
        .metatable = NULL,
        .userdata = p,
    };

    memset(result->type, '\0', type_len);
    strcpy(result->type, type);

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    if (userdata->free != NULL) {
        userdata->free(I, userdata->userdata);
    }

    allocI_free(I, userdata->type);
    allocI_free(I, userdata);
}

size_t userdataI_allocated_size(struct userdata *userdata) {
    return (sizeof(char) * (strlen(userdata->type) + 1)) + sizeof(struct userdata);
}
