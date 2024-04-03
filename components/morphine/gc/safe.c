//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"

void gcI_safe(morphine_instance_t I, struct value value) {
    I->G.safe.value = value;
}

void gcI_reset_safe(morphine_instance_t I) {
    I->G.safe.value = valueI_nil;
}
