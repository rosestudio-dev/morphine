//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

static struct userdata *create(morphine_instance_t I) {
    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata));
    (*result) = (struct userdata) {
        .is_typed = false,
        .untyped.size = 0,
        .untyped.free = NULL,
        .data = NULL,
        .mode.metatable_locked = false,
        .mode.size_locked = false,
        .metatable = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

struct userdata *userdataI_instance(morphine_instance_t I, const char *type, struct table *metatable) {
    struct usertype *usertype = usertypeI_get(I, type);
    struct usertype_info info = usertypeI_info(I, usertype);

    if (metatable == NULL && info.require_metatable) {
        throwI_error(I, "type expected metatable");
    } else if (metatable != NULL && !info.require_metatable) {
        throwI_error(I, "type unexpected metatable");
    }

    size_t rollback;
    if (metatable != NULL) {
        rollback = gcI_safe_obj(I, objectI_cast(metatable));
    }
    struct userdata *userdata = create(I);

    // config
    if (metatable != NULL) {
        gcI_safe_obj(I, objectI_cast(userdata));
    } else {
        rollback = gcI_safe_obj(I, objectI_cast(userdata));
    }

    userdata->data = allocI_uni(I, NULL, info.allocate);
    userdata->is_typed = true;
    userdata->typed.usertype = usertype;
    userdata->typed.inited = false;
    userdata->mode.metatable_locked = true;
    userdata->mode.size_locked = true;

    userdata->metatable = metatable;
    gcI_objbarrier(I, userdata, metatable);

    usertypeI_ref(I, usertype);

    if (info.init != NULL) {
        info.init(I, userdata->data);
    }

    userdata->typed.inited = true;

    gcI_reset_safe(I, rollback);

    return userdata;
}

struct userdata *userdataI_create(morphine_instance_t I, size_t size) {
    struct userdata *userdata = create(I);
    size_t rollback = gcI_safe_obj(I, objectI_cast(userdata));

    userdata->data = allocI_uni(I, NULL, size);
    userdata->untyped.size = size;

    gcI_reset_safe(I, rollback);

    return userdata;
}

struct userdata *userdataI_create_vec(
    morphine_instance_t I, size_t count, size_t size
) {
    struct userdata *userdata = create(I);

    size_t rollback = gcI_safe_obj(I, objectI_cast(userdata));

    userdata->data = allocI_vec(I, NULL, count, size);
    userdata->untyped.size = count * size;

    gcI_reset_safe(I, rollback);

    return userdata;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    morphine_userdata_free_t free_function = NULL;
    if (userdata->is_typed) {
        struct usertype_info info = usertypeI_info(I, userdata->typed.usertype);
        usertypeI_unref(I, userdata->typed.usertype);

        if (userdata->typed.inited) {
            free_function = info.free;
        }
    } else {
        free_function = userdata->untyped.free;
    }

    if (free_function != NULL) {
        free_function(I, userdata->data);
    }

    allocI_free(I, userdata->data);
    allocI_free(I, userdata);
}

void userdataI_set_free(morphine_instance_t I, struct userdata *userdata, morphine_userdata_free_t free) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->untyped.free = free;
}

void userdataI_mode_lock_metatable(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->mode.metatable_locked = true;
}

void userdataI_mode_lock_size(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->mode.size_locked = true;
}

void userdataI_resize(morphine_instance_t I, struct userdata *userdata, size_t size) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->mode.size_locked) {
        throwI_error(I, "userdata size is locked");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->data = allocI_uni(I, userdata->data, size);
    userdata->untyped.size = size;
}

void userdataI_resize_vec(morphine_instance_t I, struct userdata *userdata, size_t count, size_t size) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->mode.size_locked) {
        throwI_error(I, "userdata size is locked");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->data = allocI_vec(I, userdata->data, count, size);
    userdata->untyped.size = size;
}
