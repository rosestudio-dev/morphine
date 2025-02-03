//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"
#include "morphine/utils/array_size.h"

struct value gcI_safe(morphine_instance_t I, struct value value) {
    if (I->G.safe.values.occupied >= array_size(I->G.safe.values.stack)) {
        throwI_panic(I, "gc safe values stack is full");
    }

    if (I->G.safe.rollback.occupied == 0) {
        throwI_panic(I, "attempt recognize safe value without scope");
    }

    I->G.safe.values.stack[I->G.safe.values.occupied] = value;
    I->G.safe.values.occupied++;

    return value;
}

void gcI_safe_enter(morphine_instance_t I) {
    if (I->G.safe.rollback.occupied > array_size(I->G.safe.rollback.stack)) {
        throwI_panic(I, "gc safe rollback stack is full");
    }

    I->G.safe.rollback.stack[I->G.safe.rollback.occupied] = I->G.safe.values.occupied;
    I->G.safe.rollback.occupied++;
}

void gcI_safe_exit(morphine_instance_t I) {
    if (I->G.safe.rollback.occupied == 0) {
        throwI_panic(I, "gc safe rollback stack is empty");
    }

    I->G.safe.rollback.occupied--;
    size_t rollback = I->G.safe.rollback.stack[I->G.safe.rollback.occupied];

    size_t size = array_size(I->G.safe.values.stack);
    for (size_t i = rollback; i < size; i++) {
        I->G.safe.values.stack[i] = valueI_nil;
    }

    I->G.safe.values.occupied = rollback;
}

size_t gcI_safe_level(morphine_instance_t I) {
    return I->G.safe.rollback.occupied;
}

void gcI_safe_reset(morphine_instance_t I, size_t level) {
    if (level > I->G.safe.rollback.occupied) {
        throwI_panic(I, "cannot reset gc safe");
    }

    while (gcI_safe_level(I) > level) {
        gcI_safe_exit(I);
    }
}
