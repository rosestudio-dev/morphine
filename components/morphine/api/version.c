//
// Created by why-iskra on 26.03.2024.
//

#include "morphine/api.h"

MORPHINE_API const char *mapi_version(void) {
    return MORPHINE_VERSION;
}

MORPHINE_API int mapi_version_code(void) {
    return MORPHINE_VERSION_CODE;
}