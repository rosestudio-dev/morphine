//
// Created by whyiskra on 16.12.23.
//

#include "morphine/gc/object.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/pools.h"
#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/exception.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/reference.h"
#include "morphine/object/stream.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/vector.h"

static inline void destruct(morphine_instance_t I, struct object *object) {
    switch (object->type) {
        case OBJ_TYPE_USERDATA:
            userdataI_free(I, cast(struct userdata *, object));
            return;
        case OBJ_TYPE_STRING:
            stringI_free(I, cast(struct string *, object));
            return;
        case OBJ_TYPE_TABLE:
            tableI_free(I, cast(struct table *, object));
            return;
        case OBJ_TYPE_VECTOR:
            vectorI_free(I, cast(struct vector *, object));
            return;
        case OBJ_TYPE_CLOSURE:
            closureI_free(I, cast(struct closure *, object));
            return;
        case OBJ_TYPE_FUNCTION:
            functionI_free(I, cast(struct function *, object));
            return;
        case OBJ_TYPE_NATIVE:
            nativeI_free(I, cast(struct native *, object));
            return;
        case OBJ_TYPE_COROUTINE:
            coroutineI_free(I, cast(morphine_coroutine_t, object));
            return;
        case OBJ_TYPE_REFERENCE:
            referenceI_free(I, cast(struct reference *, object));
            return;
        case OBJ_TYPE_EXCEPTION:
            exceptionI_free(I, cast(struct exception *, object));
            return;
        case OBJ_TYPE_STREAM:
            streamI_free(I, cast(struct stream *, object));
            return;
    }

    throwI_panic(I, "unknown object type");
}

void objectI_init(morphine_instance_t I, struct object *object, enum obj_type type) {
    *object = (struct object) {
        .type = type,
        .flags.finalized = false,
        .color = OBJ_COLOR_WHITE
    };

    gcI_pools_insert(object, &I->G.pools.allocated);
}

void objectI_free(morphine_instance_t I, struct object *object) {
    throwI_danger_enter(I);
    destruct(I, object);
    throwI_danger_exit(I);
}
