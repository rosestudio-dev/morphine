//
// Created by whyiskra on 3/16/24.
//

#include <sys/time.h>
#include <stddef.h>
#include "millis.h"

uint64_t millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (((uint64_t) tv.tv_sec) * 1000) + (((uint64_t) tv.tv_usec) / 1000);
}