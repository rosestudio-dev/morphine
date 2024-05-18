//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/core/interpreter.h"
#include "morphine/object/coroutine.h"

MORPHINE_API morphine_instance_t mapi_open(struct platform platform, struct settings settings, void *data) {
    return instanceI_open(platform, settings, data);
}

MORPHINE_API void *mapi_instance_data(morphine_instance_t I) {
    return I->data;
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

MORPHINE_API morphine_instance_t mapi_instance(morphine_coroutine_t U) {
    return U->I;
}

MORPHINE_API morphine_coroutine_t mapi_coroutine(morphine_instance_t I) {
    morphine_coroutine_t U = coroutineI_create(I, valueI_object(I->env));
    coroutineI_attach(U);
    return U;
}
