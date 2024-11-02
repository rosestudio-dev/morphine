//
// Created by why-iskra on 30.03.2024.
//

#include "morphine/init/instance.h"
#include "morphine/core/instance.h"
#include "morphine/object/table.h"
#include "morphine/gc/finalizer.h"
#include "morphine/gc/control.h"
#include "stages/impl.h"

static void post_init(morphine_instance_t I) {
    tableI_mode_lock(I, I->env);

    gcI_init_finalizer(I);
    gcI_enable(I);
}

void initI_instance(morphine_instance_t I) {
    init_sio(I);
    init_env(I);
    init_localstorage(I);
    init_sharedstorage(I);
    init_metatables(I);
    init_libraries(I);

    post_init(I);
}
