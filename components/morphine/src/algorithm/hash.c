//
// Created by why-iskra on 03.10.2024.
//

#include "morphine/algorithm/hash.h"

ml_hash calchash(size_t size, const uint8_t *data) {
    ml_hash result = 0;
    for (size_t i = 0; i < size; i++) {
        result = (ml_hash) ((sizeof(result) * 8) - 1) * result + (ml_hash) data[i];
    }

    return result;
}
