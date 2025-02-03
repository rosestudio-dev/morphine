//
// Created by why-iskra on 26.03.2024.
//

#include "morphine/api.h"

MORPHINE_API const char *mapi_version_name(void) {
    return MORPHINE_VERSION_NAME;
}

MORPHINE_API ml_version mapi_version(void) {
    return MORPHINE_VERSION_CODE;
}

MORPHINE_API ml_version mapi_bytecode_version(void) {
    return MORPHINE_BYTECODE_VERSION;
}