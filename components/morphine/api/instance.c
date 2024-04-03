//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/core/interpreter.h"
#include "morphine/object/state.h"

MORPHINE_API morphine_instance_t mapi_open(struct platform platform, struct settings settings, void *userdata) {
    return instanceI_open(platform, settings, userdata);
}

MORPHINE_API void *mapi_userdata(morphine_instance_t I) {
    return I->userdata;
}

MORPHINE_API void mapi_interpreter(morphine_instance_t I) {
    interpreterI_run(I);
}

MORPHINE_API void mapi_close(morphine_instance_t I) {
    instanceI_close(I);
}

MORPHINE_API void mapi_userlibs(morphine_instance_t I, struct require_loader *table) {
    instanceI_require_table(I, table);
}

MORPHINE_API FILE *mapi_io_out(morphine_instance_t I) {
    return I->platform.io.out;
}

MORPHINE_API FILE *mapi_io_in(morphine_instance_t I) {
    return I->platform.io.in;
}

MORPHINE_API morphine_instance_t mapi_instance(morphine_state_t S) {
    return S->I;
}

MORPHINE_API morphine_state_t mapi_state(morphine_instance_t I) {
    morphine_state_t S = stateI_create(I);
    stateI_attach(S);
    return S;
}
