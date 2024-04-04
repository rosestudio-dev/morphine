//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include <setjmp.h>
#include "morphine/algorithm/crc32.h"
#include "morphine/platform.h"

struct process_state;

typedef struct function *(function_loader_t)(morphine_coroutine_t, struct process_state *);

struct function *process(
    morphine_coroutine_t,
    morphine_loader_init_t,
    morphine_loader_read_t,
    morphine_loader_finish_t,
    void *args,
    function_loader_t
);

morphine_noret void process_error(
    struct process_state *,
    const char *message
);

uint8_t process_byte(struct process_state *);
