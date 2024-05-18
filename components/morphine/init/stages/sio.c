//
// Created by why-iskra on 19.05.2024.
//

#include "../stages.h"
#include "morphine/core/instance.h"
#include "morphine/object/sio.h"

void init_sio(morphine_instance_t I) {
    I->sio.io = sioI_create(I, I->platform.sio_io_interface);
    I->sio.error = sioI_create(I, I->platform.sio_error_interface);

    sioI_open(I, I->sio.io, I->data);
    sioI_open(I, I->sio.error, I->data);
}