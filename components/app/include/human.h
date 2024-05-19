//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

char *human_size(size_t size, char *buf, size_t buf_size);
char *human_time(uint64_t time, char *buf, size_t buf_size);
