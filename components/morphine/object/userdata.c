//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"

struct userdata *userdataI_create(
    morphine_instance_t I,
    const char *name,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    if (name == NULL) {
        throwI_error(I, "Userdata name is null");
    }

    size_t name_len = strlen(name);

    if (name_len > MLIMIT_USERDATA_NAME) {
        throwI_error(I, "Native name too big");
    }

    size_t alloc_size = sizeof(struct userdata) + ((name_len + 1) * sizeof(char));
    struct userdata *result = allocI_uni(I, NULL, alloc_size);

    (*result) = (struct userdata) {
        .name = ((void *) result) + sizeof(struct userdata),
        .name_len = name_len,
        .data = p,
        .free = free,
        .mark = mark,
        .metatable = NULL,
    };

    memset(result->name, 0, (name_len + 1) * sizeof(char));
    strcpy(result->name, name);

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    if (userdata->free != NULL) {
        userdata->free(I, userdata->data);
    }

    allocI_free(I, userdata);
}
