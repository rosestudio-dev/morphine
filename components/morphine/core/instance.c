//
// Created by whyiskra on 1/21/24.
//

#include "morphine/core/instance.h"
#include "morphine/core/init.h"
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
        .states = NULL,
        .candidates = NULL,
        .interpreter_circle = 0,
        .throw = throw_prototype()
    };

    initI_vm(I);

    gcI_init_finalizer(I);
    gcI_recognize(I);
    gcI_enable(I);

    return I;
}

void instanceI_close(morphine_instance_t I) {
    gcI_destruct(I, I->G);
    I->platform.functions.free(I);
}

void instanceI_require_table(morphine_instance_t I, struct require_loader *table) {
    I->require_loader_table = table;
}
