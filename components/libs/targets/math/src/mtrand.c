//
// Created by why-iskra on 25.12.2024.
//

#include "mtrand.h"

#define UMASK (0x80000000)
#define LMASK (0x7fffffff)
#define BMASK (0x9d2c5680)
#define CMASK (0xefc60000)
#define AMTRX (0x9908b0df)
#define FFCTR (1812433253)
#define DSEED (5489)

static inline void gen_state(mtrand *mtrand, uint32_t seed) {
    mtrand->state[0] = seed;
    for (mtrand->index = 1; mtrand->index < STATE_N; mtrand->index++) {
        seed = FFCTR * (seed ^ (seed >> 30)) + (uint32_t) mtrand->index;
        mtrand->state[mtrand->index] = seed;
    }
}

mtrand mtrand_get(uint32_t seed) {
    mtrand result;
    gen_state(&result, seed);
    return result;
}

uint32_t mtrand_rand(mtrand *mtrand) {
    uint32_t mag[2] = { 0x0, AMTRX };

    if (mtrand->index >= STATE_N) {
        if (mtrand->index >= STATE_N + 1) {
            gen_state(mtrand, DSEED);
        }

        uint32_t y;
        size_t kk;
        for (kk = 0; kk < STATE_N - STATE_M; kk++) {
            y = (mtrand->state[kk] & UMASK) | (mtrand->state[kk + 1] & LMASK);
            mtrand->state[kk] = mtrand->state[kk + STATE_M] ^ (y >> 1) ^ mag[y & 0x1];
        }

        for (; kk < STATE_N - 1; kk++) {
            y = (mtrand->state[kk] & UMASK) | (mtrand->state[kk + 1] & LMASK);
            mtrand->state[kk] = mtrand->state[kk + ((size_t) (STATE_M - STATE_N))] ^ (y >> 1) ^ mag[y & 0x1];
        }

        y = (mtrand->state[STATE_N - 1] & UMASK) | (mtrand->state[0] & LMASK);
        mtrand->state[STATE_N - 1] = mtrand->state[STATE_M - 1] ^ (y >> 1) ^ mag[y & 0x1];

        mtrand->index = 0;
    }

    uint32_t y = mtrand->state[mtrand->index++];
    y ^= (y >> 11);
    y ^= (y << 7) & BMASK;
    y ^= (y << 15) & CMASK;
    y ^= (y >> 18);

    return y;
}
