//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/init/instance.h"
#include "morphine/core/instance.h"
#include "morphine/object/table.h"
#include "stages/impl.h"

static void post_init(morphine_instance_t I) {
    tableI_mode_lock(I, I->env);
}

void initI_instance(morphine_instance_t I) {
    init_sio(I);
    init_env(I);
    init_registry(I);
    init_metatables(I);
    init_functions(I);

    post_init(I);
}
