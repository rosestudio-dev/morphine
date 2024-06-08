//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/cache.h"
#include "morphine/core/instance.h"

void gcI_cache_dispose_callinfo(morphine_instance_t I, struct callinfo *callinfo) {
    callinfo->prev = I->G.cache.callinfo.pool;
    I->G.cache.callinfo.pool = callinfo;
    I->G.cache.callinfo.size++;
}

struct callinfo *gcI_cache_callinfo(morphine_instance_t I) {
    struct callinfo *callinfo = I->G.cache.callinfo.pool;

    if (callinfo != NULL) {
        I->G.cache.callinfo.pool = callinfo->prev;
        callinfo->prev = NULL;

        I->G.cache.callinfo.size--;
    }

    return callinfo;
}