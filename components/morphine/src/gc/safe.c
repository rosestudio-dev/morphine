//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"

#define safe_stack_size(I) (sizeof((I)->G.safe.stack) / sizeof(struct value))

struct value *gcI_safe(morphine_instance_t I, size_t *rollback) {
    if (I->G.safe.index >= safe_stack_size(I)) {
        throwI_panic(I, "gc safe stack is full");
    }

    if (rollback == NULL) {
        throwI_panic(I, "gc safe rollback is null");
    } else {
        *rollback = I->G.safe.index;
    }

    I->G.safe.index++;
    return I->G.safe.stack + I->G.safe.index - 1;
}

size_t gcI_safe_value(morphine_instance_t I, struct value value) {
    size_t rollback = 0;
    *gcI_safe(I, &rollback) = value;
    return rollback;
}

size_t gcI_safe_obj(morphine_instance_t I, struct object *object) {
    size_t rollback = 0;
    *gcI_safe(I, &rollback) = valueI_object(object);
    return rollback;
}

void gcI_reset_safe(morphine_instance_t I, size_t rollback) {
    if (rollback >= safe_stack_size(I)) {
        throwI_panic(I, "gc safe rollback out of bounce");
    }

    size_t size = safe_stack_size(I);

    for (size_t i = rollback; i < size; i++) {
        I->G.safe.stack[i] = valueI_nil;
    }

    I->G.safe.index = rollback;
}
