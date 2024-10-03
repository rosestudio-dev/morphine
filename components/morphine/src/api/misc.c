//
// Created by why-iskra on 03.10.2024.
//

#include "morphine/api.h"
#include "morphine/algorithm/hash.h"

MORPHINE_API ml_hash mapi_misc_hash(size_t size, const uint8_t *data) {
    return calchash(size, data);
}
