//
// Created by whyiskra on 3/23/24.
//

#include "stages.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"

void gcstageI_prepare(morphine_instance_t I) {
    if (unlikely(I->G.pools.gray != NULL || I->G.pools.white != NULL)) {
        throwI_message_panic(I, NULL, "Corrupted gc pools");
    }

    struct object *current = I->G.pools.allocated;
    while (current != NULL) {
        current->flags.mark = false;
        current = current->prev;
    }
}