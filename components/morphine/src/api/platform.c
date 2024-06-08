//
// Created by why-iskra on 08.06.2024.
//

#include "morphine/api.h"
#include "morphine/platform/conversions.h"

MORPHINE_API bool mapi_platform_str2int(const char *string, ml_integer *container) {
    return platformI_string2integer(string, container);
}

MORPHINE_API bool mapi_platform_str2dec(const char *string, ml_decimal *container) {
    return platformI_string2decimal(string, container);
}