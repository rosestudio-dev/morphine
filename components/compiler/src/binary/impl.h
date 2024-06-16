//
// Created by why-iskra on 12.06.2024.
//

#pragma once

#include <morphine.h>

#define FORMAT_TAG "morphine-binary"

#define PROB_INTEGER (-201427)
#define PROB_SIZE    (201427)
#define PROB_DECIMAL (1548.5629)

enum constant_tag {
    CONSTANT_TAG_NIL,
    CONSTANT_TAG_INT,
    CONSTANT_TAG_DEC,
    CONSTANT_TAG_STR,
    CONSTANT_TAG_FUN,
    CONSTANT_TAG_BOOL
};

void binary_to(morphine_coroutine_t);
void binary_from(morphine_coroutine_t);
