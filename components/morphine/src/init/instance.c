//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/init/instance.h"
#include "stages/impl.h"

void initI_instance(morphine_instance_t I) {
    init_sio(I);
    init_env(I);
    init_registry(I);
    init_metatables(I);
    init_functions(I);
}
