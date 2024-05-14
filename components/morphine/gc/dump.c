//
// Created by why-iskra on 31.03.2024.
//

#include "morphine/gc/dump.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/userdata.h"

static const char *color(enum obj_color color) {
    switch (color) {
        case OBJ_COLOR_BLACK:
            return "B";
        case OBJ_COLOR_GREY:
            return "G";
        case OBJ_COLOR_WHITE:
            return "W";
        case OBJ_COLOR_RED:
            return "R";
    }

    return "?";
}

static void object(morphine_instance_t I, struct object *obj) {
    FILE *out = I->platform.io.out;
    if (obj == NULL) {
        fprintf(out, "null");
    } else if (obj->type == OBJ_TYPE_STRING) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            color(obj->color),
            obj->flags.finalized ? "F" : " ",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct string *) obj)->chars
        );
    } else if (obj->type == OBJ_TYPE_FUNCTION) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            color(obj->color),
            obj->flags.finalized ? "F" : " ",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct function *) obj)->name
        );
    } else if (obj->type == OBJ_TYPE_NATIVE) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            color(obj->color),
            obj->flags.finalized ? "F" : " ",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct native *) obj)->name
        );
    } else if (obj->type == OBJ_TYPE_USERDATA) {
        fprintf(
            out, "(%s%s) %s %p '%s'",
            color(obj->color),
            obj->flags.finalized ? "F" : " ",
            valueI_type2string(I, valueI_object(obj).type),
            obj,
            ((struct userdata *) obj)->name
        );
    } else {
        fprintf(
            out, "(%s%s) %s %p",
            color(obj->color),
            obj->flags.finalized ? "F" : " ",
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
        fprintf(out, "@   - ");
        object(I, current);
        fprintf(out, "\n");
        current = current->prev;
    }
}

void gcI_dump(morphine_instance_t I) {
    FILE *out = I->platform.io.out;

    if (out == NULL) {
        return;
    }

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
        case GC_STATUS_RESOLVE:
            status = "resolve";
            break;
        case GC_STATUS_SWEEP:
            status = "sweep";
            break;
    }

    fprintf(out, "@ Garbage collector dump\n");
    fprintf(out, "@ Status:                 %s\n", status);
    fprintf(out, "@ Stats:\n");
    fprintf(out, "@   - Debt:               %zu\n", I->G.stats.debt);
    fprintf(out, "@   - Previous allocated: %zu\n", I->G.stats.prev_allocated);
    fprintf(out, "@ Bytes:\n");
    fprintf(out, "@   - Allocated:          %zu\n", I->G.bytes.allocated);
    fprintf(out, "@   - Max allocated:      %zu\n", I->G.bytes.max_allocated);
    fprintf(out, "@ Settings:\n");
    fprintf(out, "@   - Limit:              %zu\n", I->G.settings.limit);
    fprintf(out, "@   - Grow:               %"PRIu16"\n", I->G.settings.grow * 10);
    fprintf(out, "@   - Deal:               %"PRIu16"\n", I->G.settings.deal * 10);
    fprintf(out, "@   - Pause:              %zu\n", I->G.settings.pause);
    fprintf(out, "@ Finalizer:\n");
    fprintf(out, "@   - Work:               %s\n", I->G.finalizer.work ? "yes" : "no");
    fprintf(out, "@   - Candidate:          ");
    object(I, I->G.finalizer.candidate);
    fprintf(out, "\n");
    fprintf(out, "@   - State:              ");
    object(I, (struct object *) I->G.finalizer.coroutine);
    fprintf(out, "\n");
    fprintf(out, "@ Safe:\n");
    for (size_t i = 0; i < sizeof(I->G.safe.stack) / sizeof(struct value); i++) {
        fprintf(out, "@   - Value:              ");
        value(I, I->G.safe.stack[i]);
        fprintf(out, "\n");
    }
    fprintf(out, "@\n");
    fprintf(out, "@ Pool 'allocated':\n");
    pool(I, I->G.pools.allocated);
    fprintf(out, "@ Pool 'grey':\n");
    pool(I, I->G.pools.grey);
    fprintf(out, "@ Pool 'black':\n");
    pool(I, I->G.pools.black);
    fprintf(out, "@ Pool 'black coroutines':\n");
    pool(I, I->G.pools.black_coroutines);
    fprintf(out, "@ Pool 'sweep':\n");
    pool(I, I->G.pools.sweep);
    fprintf(out, "@ Pool 'finalize':\n");
    pool(I, I->G.pools.finalize);
    fprintf(out, "\n");
}
