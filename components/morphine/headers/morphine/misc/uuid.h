//
// Created by why-iskra on 09.04.2024.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

struct uuid {
    uint64_t most_significant_bits;
    uint64_t least_significant_bits;
};

bool uuidI_equal(struct uuid a, struct uuid b);
