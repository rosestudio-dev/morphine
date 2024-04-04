//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "process.h"
#include "morphine/algorithm/crc32.h"

#define FORMAT_TAG "morphine-bout"

struct binary_data {
    struct crc32_buf crc;
    struct proto **created_protos;
};

struct proto *binary(
    morphine_coroutine_t,
    struct process_state *
);
