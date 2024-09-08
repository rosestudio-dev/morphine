//
// Created by whyiskra on 1/21/24.
//

#include "morphine/core/instance.h"
#include "morphine/init/instance.h"
#include "morphine/gc/finalizer.h"
#include "morphine/gc/control.h"
#include "morphine/object/sio.h"

morphine_instance_t instanceI_open(morphine_platform_t platform, morphine_settings_t settings, void *data) {
    if (sizeof(struct instance) >= settings.gc.limit_bytes) {
        platform.functions.signal(NULL);
    }

    morphine_instance_t I = platform.functions.malloc(data, sizeof(struct instance));

    if (I == NULL) {
        platform.functions.signal(NULL);
    }

    *I = (struct instance) {
        .platform = platform,
        .settings = settings,
        .E = interpreterI_prototype(),
        .libraries = librariesI_prototype(),
        .usertypes = usertypeI_prototype(),
        .data = data,
        .env = NULL,
        .localstorage = NULL
    };

    gcI_prototype(I, sizeof(struct instance));

    initI_instance(I);

    gcI_init_finalizer(I);
    gcI_enable(I);

    return I;
}

void instanceI_close(morphine_instance_t I) {
    sioI_close(I, I->sio.io, true);
    sioI_close(I, I->sio.error, true);

    gcI_destruct(I, I->G);

    librariesI_free(I, &I->libraries);
    usertypeI_free(I, &I->usertypes);

    I->platform.functions.free(I->data, I);
}
