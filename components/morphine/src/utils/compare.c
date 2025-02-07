//
// Created by why-iskra on 06.10.2024.
//

#include "morphine/utils/compare.h"
#include <stdint.h>

int arrcmp(const void *a, const void *b, size_t a_size, size_t b_size) {
    if (a_size != b_size) {
        return smpcmp(a_size, b_size);
    }

    int flag = 0;
    for (size_t i = 0; flag == 0 && i < a_size && i < b_size; i++) {
        uint8_t ca = *((const uint8_t *) (a + i));
        uint8_t cb = *((const uint8_t *) (b + i));
        flag = smpcmp(ca, cb);
    }

    return flag;
}
