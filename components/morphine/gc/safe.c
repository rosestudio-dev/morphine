//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"

void gcI_safe(morphine_instance_t I, struct value value) {
    if (I->G.safe.index >= sizeof(I->G.safe.stack) / sizeof(struct value)) {
        throwI_panic(I, "GC safe stack is full");
    }

    I->G.safe.stack[I->G.safe.index ++] = value;
}

void gcI_reset_safe(morphine_instance_t I) {
    size_t size = sizeof(I->G.safe.stack) / sizeof(struct value);

    for (size_t i = 0; i < size; i++) {
        I->G.safe.stack[i] = valueI_nil;
    }

    I->G.safe.index = 0;
}
