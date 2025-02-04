//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/userdata.h"
#include "morphine/algorithm/hash.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/utils/compare.h"
#include <string.h>

static struct userdata *create(morphine_instance_t I) {
    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata));
    (*result) = (struct userdata) {
        .is_typed = false,
        .untyped.size = 0,
        .untyped.destructor = NULL,
        .data = NULL,
        .lock.destructor = false,
        .lock.metatable = false,
        .lock.size = false,
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

    gcI_safe_enter(I);
    if (metatable != NULL) {
        gcI_safe(I, valueI_object(metatable));
    }
    struct userdata *userdata = create(I);

    // config
    gcI_safe(I, valueI_object(userdata));

    userdata->data = allocI_uni(I, NULL, info.allocate);
    userdata->is_typed = true;
    userdata->typed.usertype = usertype;
    userdata->typed.inited = false;
    userdata->lock.destructor = true;
    userdata->lock.metatable = true;
    userdata->lock.size = true;

    userdata->metatable = metatable;
    gcI_objbarrier(I, userdata, metatable);

    usertypeI_ref(I, usertype);

    if (info.constructor != NULL) {
        info.constructor(I, userdata->data);
    }

    userdata->typed.inited = true;

    gcI_safe_exit(I);

    return userdata;
}

struct userdata *userdataI_create(morphine_instance_t I, size_t size) {
    gcI_safe_enter(I);
    struct userdata *userdata = gcI_safe_obj(I, userdata, create(I));

    userdata->data = allocI_uni(I, NULL, size);
    userdata->untyped.size = size;

    gcI_safe_exit(I);

    return userdata;
}

struct userdata *userdataI_create_vec(morphine_instance_t I, size_t count, size_t size) {
    gcI_safe_enter(I);
    struct userdata *userdata = gcI_safe_obj(I, userdata, create(I));

    userdata->data = allocI_vec(I, NULL, count, size);
    userdata->untyped.size = count * size;

    gcI_safe_exit(I);

    return userdata;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    morphine_userdata_destructor_t destructor = NULL;
    if (userdata->is_typed) {
        struct usertype_info info = usertypeI_info(I, userdata->typed.usertype);
        usertypeI_unref(I, userdata->typed.usertype);

        if (userdata->typed.inited) {
            destructor = info.destructor;
        }
    } else {
        destructor = userdata->untyped.destructor;
    }

    if (destructor != NULL) {
        destructor(I, userdata->data);
    }

    allocI_free(I, userdata->data);
    allocI_free(I, userdata);
}

void userdataI_set_destructor(
    morphine_instance_t I,
    struct userdata *userdata,
    morphine_userdata_destructor_t destructor
) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    if (userdata->lock.destructor) {
        throwI_error(I, "userdata destructor is locked");
    }

    userdata->untyped.destructor = destructor;
}

void userdataI_lock_destructor(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->lock.destructor = true;
}

void userdataI_lock_metatable(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->lock.metatable = true;
}

void userdataI_lock_size(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->lock.size = true;
}

void userdataI_resize(morphine_instance_t I, struct userdata *userdata, size_t size) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->lock.size) {
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

    if (userdata->lock.size) {
        throwI_error(I, "userdata size is locked");
    }

    if (userdata->is_typed) {
        throwI_error(I, "userdata is typed");
    }

    userdata->data = allocI_vec(I, userdata->data, count, size);
    userdata->untyped.size = size;
}

int userdataI_compare(morphine_instance_t I, struct userdata *a, struct userdata *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "userdata is null");
    }

    int raw_compared = smpcmp(a, b);

    if (!a->is_typed || !b->is_typed) {
        return raw_compared;
    }

    if (!usertypeI_eq(a->typed.usertype, b->typed.usertype)) {
        return raw_compared;
    }

    morphine_userdata_compare_t compare = usertypeI_info(I, a->typed.usertype).compare;

    if (compare == NULL) {
        return raw_compared;
    }

    return compare(I, a->data, b->data);
}

ml_hash userdataI_hash(morphine_instance_t I, struct userdata *userdata) {
    if (userdata == NULL) {
        throwI_error(I, "userdata is null");
    }

    if (userdata->is_typed) {
        struct usertype_info info = usertypeI_info(I, userdata->typed.usertype);

        if (info.hash == NULL) {
            return calchash(userdata->data, info.allocate);
        } else {
            return info.hash(I, userdata->data);
        }
    }

    return calchash(userdata->data, userdata->untyped.size);
}
