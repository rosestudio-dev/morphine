//
// Created by why-iskra on 03.10.2024.
//

#include "morphine/api.h"
#include "morphine/algorithm/hash.h"

MORPHINE_API ml_hash mapi_misc_hash(const uint8_t *data, size_t size) {
    return calchash(data, size);
}
