//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/status.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"

bool gcI_isenabled(morphine_instance_t I) {
    return I->G.enabled;
}

const char *gcI_status(morphine_instance_t I) {
    switch (I->G.status) {
        case GC_STATUS_IDLE:
            return "idle";
        case GC_STATUS_PREPARE:
            return "prepare";
        case GC_STATUS_INCREMENT:
            return "increment";
        case GC_STATUS_FINALIZE_PREPARE:
            return "finalize_prepare";
        case GC_STATUS_FINALIZE_INCREMENT:
            return "finalize_increment";
        case GC_STATUS_SWEEP:
            return "sweep";
    }

    throwI_message_panic(I, NULL, "Unsupported gc status");
}

bool gcI_isrunning(morphine_instance_t I) {
    return I->G.status != GC_STATUS_IDLE;
}
