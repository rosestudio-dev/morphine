//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "morphine/core/instance.h"
#include "morphine/object/table.h"

void init_registry(morphine_instance_t I) {
    I->registry = tableI_create(I);
}
