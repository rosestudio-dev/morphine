//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object.h"
#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/userdata.h"
#include "morphine/object/closure.h"
#include "morphine/object/proto.h"
#include "morphine/object/reference.h"
#include "morphine/object/native.h"
#include "morphine/object/iterator.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"

void objectI_init(morphine_instance_t I, struct object *object, enum obj_type type) {
    *object = (struct object) {
        .type = type,
        .flags.mark = false,
        .flags.finalized = false,
        .prev = I->G.pools.allocated
    };

    I->G.pools.allocated = object;

    if (I->G.stats.debt < SIZE_MAX) {
        I->G.stats.debt ++;
    }
}

void objectI_free(morphine_instance_t I, struct object *object) {
    if (I->G.stats.debt > 0) {
        I->G.stats.debt --;
    }

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
        case OBJ_TYPE_CLOSURE:
            closureI_free(I, cast(struct closure *, object));
            return;
        case OBJ_TYPE_PROTO:
            protoI_free(I, cast(struct proto *, object));
            return;
        case OBJ_TYPE_NATIVE:
            nativeI_free(I, cast(struct native *, object));
            return;
        case OBJ_TYPE_STATE:
            stateI_free(I, cast(morphine_state_t, object));
            return;
        case OBJ_TYPE_REFERENCE:
            referenceI_free(I, cast(struct reference *, object));
            return;
        case OBJ_TYPE_ITERATOR:
            iteratorI_free(I, cast(struct iterator *, object));
            return;
    }

    throwI_panic(I, "Unknown object type");
}
