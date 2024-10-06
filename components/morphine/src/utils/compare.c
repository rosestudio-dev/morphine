//
// Created by why-iskra on 06.10.2024.
//

#include "morphine/utils/compare.h"
#include "morphine/utils/overflow.h"
#include "morphine/core/throw.h"

int arrcmp(morphine_instance_t I, const void *a, const void *b, size_t a_size, size_t b_size, size_t mul) {
    a_size = overflow_op_mul(a_size, mul, SIZE_MAX, throwI_error(I, "compare size overflow"));
    b_size = overflow_op_mul(b_size, mul, SIZE_MAX, throwI_error(I, "compare size overflow"));

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
