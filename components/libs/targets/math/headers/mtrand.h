//
// Created by why-iskra on 25.12.2024.
//

#pragma once

// Mersenne Twister PRNG Algorithm

#include <stdint.h>
#include <stddef.h>

#define STATE_N (624)
#define STATE_M (397)

typedef struct {
    uint32_t state[STATE_N];
    size_t index;
} mtrand;

mtrand mtrand_get(uint32_t);
uint32_t mtrand_rand(mtrand *);
