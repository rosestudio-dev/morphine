//
// Created by whyiskra on 1/21/24.
//

#include "morphine/core/instance.h"
#include "morphine/core/init.h"
#include "morphine/core/stack.h"
#include "morphine/gc/finalizer.h"
#include "morphine/gc/control.h"

static struct throw throw_prototype(void) {
    return (struct throw) {
        .inited = false,
        .cause_state = NULL,
        .is_message = false,
        .result.value = valueI_nil,
    };
}

morphine_instance_t instanceI_open(struct platform platform, struct params params, void *userdata) {
    if (sizeof(struct instance) >= params.gc.limit_bytes) {
        platform.functions.signal(NULL);
    }

    morphine_instance_t I = platform.functions.malloc(sizeof(struct instance));

    if (I == NULL) {
        platform.functions.signal(NULL);
    }

    *I = (struct instance) {
        .platform = platform,
        .params = params,
        .G = gcI_init(params, sizeof(struct instance)),
        .require_loader_table = NULL,
        .userdata = userdata,
        .env = NULL,
        .registry = NULL,
        .state_finalizer = NULL,
        .states = NULL,
        .candidates = NULL,
        .interpreter_circle = 0,
        .throw = throw_prototype(),
    };

    initI_vm(I);

    gcI_init_finalizer(I);
    gcI_enable(I);

    return I;
}

static void free_objects(morphine_instance_t I, struct object *pool) {
    struct object *current = pool;

    while (current != NULL) {
        struct object *prev = current->prev;
        objectI_free(I, current);
        current = prev;
    }
}

void instanceI_close(morphine_instance_t I) {
    free_objects(I, I->G.pools.allocated);
    free_objects(I, I->G.pools.gray);
    free_objects(I, I->G.pools.white);
    free_objects(I, I->G.pools.finalize);

    if (I->G.finalizer.candidate != NULL) {
        objectI_free(I, I->G.finalizer.candidate);
    }

    {
        struct callinfo *current = I->G.callinfo_trash;
        while (current != NULL) {
            struct callinfo *prev = current->prev;
            stackI_callinfo_free(I, current);

            current = prev;
        }
    }

    I->platform.functions.free((struct instance *) I);
}

void instanceI_require_table(morphine_instance_t I, struct require_loader *table) {
    I->require_loader_table = table;
}
