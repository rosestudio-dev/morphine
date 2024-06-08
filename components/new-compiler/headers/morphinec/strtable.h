//
// Created by why-iskra on 22.05.2024.
//

#pragma once

#include <morphine.h>

typedef size_t strtable_index_t;

struct strtable_entry {
    const char *string;
    size_t size;
};

void strtable(morphine_coroutine_t);
strtable_index_t strtable_record(morphine_coroutine_t, const char *, size_t);
bool strtable_has(morphine_coroutine_t, strtable_index_t);
struct strtable_entry strtable_get(morphine_coroutine_t, strtable_index_t);
