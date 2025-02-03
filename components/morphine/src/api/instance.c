//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/core/interpreter.h"
#include "morphine/object/coroutine.h"

MORPHINE_API morphine_instance_t mapi_open(morphine_platform_t P, morphine_settings_t S, void *data) {
    return instanceI_open(P, S, data);
}

MORPHINE_API void *mapi_instance_data(morphine_instance_t I) {
    return I->data;
}

MORPHINE_API void mapi_interpreter(morphine_instance_t I) {
    interpreterI_run(I);
}

MORPHINE_API bool mapi_interpreter_step(morphine_instance_t I) {
    return interpreterI_step(I);
}

MORPHINE_API void mapi_close(morphine_instance_t I) {
    instanceI_close(I);
}

MORPHINE_API morphine_instance_t mapi_instance(morphine_coroutine_t U) {
    return U->I;
}

MORPHINE_API morphine_coroutine_t mapi_coroutine(morphine_instance_t I) {
    return I->main;
}
