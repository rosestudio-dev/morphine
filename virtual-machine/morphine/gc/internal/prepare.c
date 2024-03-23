//
// Created by why on 3/23/24.
//

#include "functions.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"

void gcf_prepare(morphine_instance_t I) {
    if (morphinem_unlikely(I->G.pools.gray != NULL || I->G.pools.white != NULL)) {
        throwI_message_panic(I, NULL, "Corrupted gc pools");
    }

    I->G.pools.gray = NULL;
    I->G.pools.white = NULL;

    {
        struct object *current = I->G.pools.allocated;
        while (current != NULL) {
            current->flags.mark = false;
            current = current->prev;
        }
    }
}