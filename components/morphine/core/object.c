//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/object.h"
#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/userdata.h"
#include "morphine/object/closure.h"
#include "morphine/object/proto.h"
#include "morphine/object/reference.h"
#include "morphine/object/native.h"
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
}

void objectI_free(morphine_instance_t I, struct object *object) {
    switch (object->type) {
        case OBJ_TYPE_USERDATA:
            userdataI_free(I, morphinem_cast(struct userdata *, object));
            return;
        case OBJ_TYPE_STRING:
            stringI_free(I, morphinem_cast(struct string *, object));
            return;
        case OBJ_TYPE_TABLE:
            tableI_free(I, morphinem_cast(struct table *, object));
            return;
        case OBJ_TYPE_CLOSURE:
            closureI_free(I, morphinem_cast(struct closure *, object));
            return;
        case OBJ_TYPE_PROTO:
            protoI_free(I, morphinem_cast(struct proto *, object));
            return;
        case OBJ_TYPE_NATIVE:
            nativeI_free(I, morphinem_cast(struct native *, object));
            return;
        case OBJ_TYPE_STATE:
            stateI_free(I, morphinem_cast(morphine_state_t, object));
            return;
        case OBJ_TYPE_REFERENCE:
            referenceI_free(I, morphinem_cast(struct reference *, object));
            return;
    }

    throwI_message_panic(I, NULL, "Unknown object type");
}
