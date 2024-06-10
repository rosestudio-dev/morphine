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

struct strtable;

struct strtable *strtable(morphine_coroutine_t);
struct strtable *get_strtable(morphine_coroutine_t);

strtable_index_t strtable_record(morphine_coroutine_t, struct strtable *, const char *, size_t);
bool strtable_has(struct strtable *, strtable_index_t);
struct strtable_entry strtable_get(morphine_coroutine_t, struct strtable *, strtable_index_t);
