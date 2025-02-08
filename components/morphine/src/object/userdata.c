//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/userdata.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/utils/compare.h"
#include <string.h>

static struct userdata *create(morphine_instance_t I) {
    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata));
    (*result) = (struct userdata) {
        .inited = false,
        .is_typed = false,
        .untyped.size = 0,
        .untyped.destructor = NULL,
        .data = NULL,
        .metatable = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

struct userdata *userdataI_instance(morphine_instance_t I, const char *type_name) {
    struct usertype *type = usertypeI_get(I, type_name);

    gcI_safe_enter(I);
    struct userdata *userdata = create(I);

    // config
    gcI_safe(I, valueI_object(userdata));

    userdata->data = allocI_uni(I, NULL, type->allocate);
    userdata->typed = type;
    userdata->is_typed = true;
    userdata->metatable = gcI_objbarrier(I, userdata, userdata->metatable);

    if (type->constructor != NULL) {
        type->constructor(I, userdata->data);
    }

    userdata->inited = true;
    gcI_safe_exit(I);

    return userdata;
}

static void create_untyped(
    morphine_instance_t I,
    struct userdata *userdata,
    size_t size,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor
) {
    userdata->is_typed = false;
    userdata->untyped.size = size;
    userdata->untyped.destructor = destructor;

    if (constructor != NULL) {
        constructor(I, userdata->data);
    }

    userdata->inited = true;
}

struct userdata *userdataI_create_uni(
    morphine_instance_t I,
    size_t size,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor
) {
    gcI_safe_enter(I);
    struct userdata *userdata = gcI_safe_obj(I, userdata, create(I));
    userdata->data = allocI_uni(I, NULL, size);
    create_untyped(I, userdata, size, constructor, destructor);
    gcI_safe_exit(I);

    return userdata;
}

struct userdata *userdataI_create_vec(
    morphine_instance_t I,
    size_t count,
    size_t size,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor
) {
    gcI_safe_enter(I);
    struct userdata *userdata = gcI_safe_obj(I, userdata, create(I));
    userdata->data = allocI_vec(I, NULL, count, size);
    create_untyped(I, userdata, size * count, constructor, destructor);
    gcI_safe_exit(I);

    return userdata;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    mfunc_destructor_t destructor;
    if (userdata->is_typed) {
        destructor = userdata->typed->destructor;
    } else {
        destructor = userdata->untyped.destructor;
    }

    if (userdata->inited && destructor != NULL) {
        destructor(I, userdata->data);
        userdata->inited = false;
    }

    allocI_free(I, userdata->data);
    allocI_free(I, userdata);
}

void *userdataI_pointer(morphine_instance_t I, struct userdata *userdata, const char *type) {
    if (userdata->is_typed && (type == NULL || strcmp(userdata->typed->name, type) != 0)) {
        throwI_error(I, "userdata type doesn't match");
    } else if (!userdata->is_typed && type != NULL) {
        throwI_error(I, "untyped userdata");
    }

    return userdata->data;
}
