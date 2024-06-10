//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

static struct userdata *create(
    morphine_instance_t I,
    const char *name
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

    char *result_name = ((void *) result) + sizeof(struct userdata);
    (*result) = (struct userdata) {
        .name = result_name,
        .name_len = name_len,
        .size = 0,
        .data = NULL,
        .free = NULL,
        .mode.metatable_locked = false,
        .metatable = NULL,
    };

    memcpy(result_name, name, name_len * sizeof(char));
    result_name[name_len] = '\0';

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

struct userdata *userdataI_create(
    morphine_instance_t I,
    const char *name,
    size_t size
) {
    struct userdata *userdata = create(I, name);

    size_t rollback = gcI_safe_obj(I, objectI_cast(userdata));

    userdata->data = allocI_uni(I, NULL, size);
    userdata->size = size;

    gcI_reset_safe(I, rollback);

    return userdata;
}

struct userdata *userdataI_create_vec(
    morphine_instance_t I,
    const char *name,
    size_t count,
    size_t size
) {
    struct userdata *userdata = create(I, name);

    size_t rollback = gcI_safe_obj(I, objectI_cast(userdata));

    userdata->data = allocI_vec(I, NULL, count, size);
    userdata->size = size;

    gcI_reset_safe(I, rollback);

    return userdata;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    if (userdata->free != NULL) {
        userdata->free(I, userdata->data);
    }

    allocI_free(I, userdata->data);
    allocI_free(I, userdata);
}

void userdataI_set_free(morphine_instance_t I, struct userdata *userdata, morphine_free_t free) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    userdata->free = free;
}

void userdataI_mode_lock_metatable(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    userdata->mode.metatable_locked = true;
}

void userdataI_resize(morphine_instance_t I, struct userdata *userdata, size_t size) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    userdata->data = allocI_uni(I, userdata->data, size);
    userdata->size = size;
}

void userdataI_resize_vec(morphine_instance_t I, struct userdata *userdata, size_t count, size_t size) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    userdata->data = allocI_vec(I, userdata->data, count, size);
    userdata->size = size;
}
