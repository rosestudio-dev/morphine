//
// Created by why-iskra on 08.06.2024.
//

#include "morphine/api.h"
#include "morphine/platform/convert.h"

MORPHINE_API bool mapi_platform_str2int(const char *string, ml_integer *container, ml_size base) {
    return platformI_string2integer(string, container, base);
}

MORPHINE_API bool mapi_platform_str2dec(const char *string, ml_decimal *container) {
    return platformI_string2decimal(string, container);
}