//
// Created by why-iskra on 31.03.2024.
//

#include "morphine/gc/dump.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/userdata.h"

static void object(morphine_instance_t I, struct object *obj) {
    FILE *out = I->platform.io.out;
    if (obj == NULL) {
        fprintf(out, "null");
    } else if (obj->type == OBJ_TYPE_STRING) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            obj->flags.mark ? "+" : "-",
            obj->flags.finalized ? "l" : "f",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct string *) obj)->chars
        );
    } else if (obj->type == OBJ_TYPE_PROTO) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            obj->flags.mark ? "+" : "-",
            obj->flags.finalized ? "l" : "f",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct proto *) obj)->name
        );
    } else if (obj->type == OBJ_TYPE_NATIVE) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            obj->flags.mark ? "+" : "-",
            obj->flags.finalized ? "l" : "f",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct native *) obj)->name
        );
    } else if (obj->type == OBJ_TYPE_USERDATA) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            obj->flags.mark ? "+" : "-",
            obj->flags.finalized ? "l" : "f",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct userdata *) obj)->type
        );
    } else {
        fprintf(
            out, "(%s%s) %s %p",
            obj->flags.mark ? "+" : "-",
            obj->flags.finalized ? "l" : "f",
            valueI_type2string(I, valueI_object(obj).type),
            obj
        );
    }
}

static void value(morphine_instance_t I, struct value value) {
    FILE *out = I->platform.io.out;
    struct object *obj = valueI_safe_as_object(value, NULL);
    if (obj == NULL) {
        fprintf(out, "(value) %s", valueI_type2string(I, value.type));
    } else {
        object(I, obj);
    }
}

static void pool(morphine_instance_t I, struct object *pool) {
    FILE *out = I->platform.io.out;
    struct object *current = pool;
    while (current != NULL) {
        fprintf(out, "  - ");
        object(I, current);
        fprintf(out, "\n");
        current = current->prev;
    }
}

void gcI_dump(morphine_instance_t I) {
    const char *status = "unknown";
    switch (I->G.status) {
        case GC_STATUS_IDLE:
            status = "idle";
            break;
        case GC_STATUS_PREPARE:
            status = "prepare";
            break;
        case GC_STATUS_INCREMENT:
            status = "increment";
            break;
        case GC_STATUS_FINALIZE_PREPARE:
            status = "finalize_prepare";
            break;
        case GC_STATUS_FINALIZE_INCREMENT:
            status = "finalize_increment";
            break;
        case GC_STATUS_SWEEP:
            status = "sweep";
            break;
    }

    FILE *out = I->platform.io.out;

    fprintf(out, "Garbage collector dump\n");
    fprintf(out, "Status:                 %s\n", status);
    fprintf(out, "Bytes:\n");
    fprintf(out, "  - Allocated:          %zu\n", I->G.bytes.allocated);
    fprintf(out, "  - Previous allocated: %zu\n", I->G.bytes.prev_allocated);
    fprintf(out, "  - Max allocated:      %zu\n", I->G.bytes.max_allocated);
    fprintf(out, "  - Started:            %zu\n", I->G.bytes.started);
    fprintf(out, "Settings:\n");
    fprintf(out, "  - Limit:              %zu\n", I->G.settings.limit_bytes);
    fprintf(out, "  - Start:              %zu\n", I->G.settings.start);
    fprintf(out, "  - Grow:               %zu\n", I->G.settings.grow);
    fprintf(out, "  - Deal:               %zu\n", I->G.settings.deal);
    fprintf(out, "Finalizer:\n");
    fprintf(out, "  - Work:               %s\n", I->G.finalizer.work ? "yes" : "no");
    fprintf(out, "  - Candidate:          ");
    object(I, I->G.finalizer.candidate);
    fprintf(out, "\n");
    fprintf(out, "  - State:              ");
    object(I, (struct object *) I->G.finalizer.state);
    fprintf(out, "\n");
    fprintf(out, "Safe:\n");
    fprintf(out, "  - Value:              ");
    value(I, I->G.safe.value);
    fprintf(out, "\n");
    fprintf(out, "Pool 'allocated':\n");
    pool(I, I->G.pools.allocated);
    fprintf(out, "Pool 'gray':\n");
    pool(I, I->G.pools.gray);
    fprintf(out, "Pool 'white':\n");
    pool(I, I->G.pools.white);
    fprintf(out, "Pool 'finalize':\n");
    pool(I, I->G.pools.finalize);
    fprintf(out, "\n");
}
