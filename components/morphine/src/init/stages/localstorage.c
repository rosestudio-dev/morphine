//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "morphine/core/instance.h"
#include "morphine/object/table.h"

void init_localstorage(morphine_instance_t I) {
    I->localstorage = tableI_create(I);
}
