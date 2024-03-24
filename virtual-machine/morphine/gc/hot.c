//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/hot.h"
#include "morphine/core/instance.h"

void gcI_dispose_callinfo(morphine_instance_t I, struct callinfo *callinfo) {
    callinfo->prev = I->G.callinfo_trash;
    I->G.callinfo_trash = callinfo;
}

struct callinfo *gcI_hot_callinfo(morphine_instance_t I) {
    struct callinfo *callinfo = I->G.callinfo_trash;

    if (callinfo != NULL) {
        I->G.callinfo_trash = callinfo->prev;
    }

    return callinfo;
}