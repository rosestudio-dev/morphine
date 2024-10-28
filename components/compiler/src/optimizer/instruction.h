//
// Created by why-iskra on 29.09.2024.
//

#pragma once

#include <morphine.h>

enum instruction_type {
    IT_CONTROL,
    IT_CONSUMER,
    IT_PROCESSOR,
    IT_PRODUCER,
};

struct instruction {
    enum instruction_type type;

    bool has_dest;
    size_t src_count;

    size_t dest;
    size_t srcs[MORPHINE_INSTRUCTION_ARGS_COUNT];

    morphine_instruction_t origin;
};

struct instruction instruction_convert(morphine_coroutine_t, morphine_instruction_t);
morphine_instruction_t instruction_build(morphine_coroutine_t, struct instruction);
