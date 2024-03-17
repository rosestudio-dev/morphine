//
// Created by whyiskra on 3/16/24.
//

#include <stdio.h>
#include "human.h"

char *human_size(size_t bytes, char *buf, size_t buf_size) {
    char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);
    int i;

    for (i = 0; i < length; i++) {
        if (bytes < 1024) {
            break;
        }

        bytes >>= 10;
    }

    snprintf(buf, buf_size, "%zu%s", bytes, suffix[i]);

    return buf;
}

char *human_time(uint64_t time, char *buf, size_t buf_size) {
    char *suffix[] = {"ms", "s", "m", "h"};
    uint64_t div[] = { 1000, 60, 60, 60 };

    char length = sizeof(suffix) / sizeof(suffix[0]);
    int i;

    for (i = 0; i < length; i++) {
        if (time < div[i]) {
            break;
        }

        time /= div[i];
    }

    snprintf(buf, buf_size, "%zu%s", time, suffix[i]);

    return buf;
}