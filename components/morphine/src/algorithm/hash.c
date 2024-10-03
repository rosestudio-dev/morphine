//
// Created by why-iskra on 03.10.2024.
//

#include "morphine/algorithm/hash.h"

ml_hash calchash(size_t size, const uint8_t *data) {
    ml_hash h = 0;
    for (size_t i = 0; i < size; i++) {
        h = 31 * h + (ml_hash) data[i];
    }

    return h;
}
