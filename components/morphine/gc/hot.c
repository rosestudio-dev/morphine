//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/hot.h"
#include "morphine/core/instance.h"

void gcI_dispose_callinfo(morphine_instance_t I, struct callinfo *callinfo) {
    callinfo->prev = I->G.trash.callinfo;
    I->G.trash.callinfo = callinfo;
}

struct callinfo *gcI_hot_callinfo(morphine_instance_t I) {
    struct callinfo *callinfo = I->G.trash.callinfo;

    if (callinfo != NULL) {
        I->G.trash.callinfo = callinfo->prev;
        callinfo->prev = NULL;
    }

    return callinfo;
}