//
// Created by why-iskra on 22.05.2024.
//

#pragma once

#include <morphine.h>

typedef size_t morphinec_strtable_index_t;

struct morphinec_strtable_entry {
    const char *string;
    size_t size;
};

struct morphinec_strtable;

struct morphinec_strtable *mcapi_push_strtable(morphine_coroutine_t);
struct morphinec_strtable *mcapi_get_strtable(morphine_coroutine_t);

bool mcapi_strtable_has(struct morphinec_strtable *, morphinec_strtable_index_t);

morphinec_strtable_index_t mcapi_strtable_record(
    morphine_coroutine_t,
    struct morphinec_strtable *,
    const char *,
    size_t
);

struct morphinec_strtable_entry mcapi_strtable_access(
    morphine_coroutine_t,
    struct morphinec_strtable *,
    morphinec_strtable_index_t
);
