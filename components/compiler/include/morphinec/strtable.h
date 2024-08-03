//
// Created by why-iskra on 22.05.2024.
//

#pragma once

#include <morphine.h>

#define MC_STRTABLE_USERDATA_TYPE "morphinec-strtable"

typedef ml_size mc_strtable_index_t;

struct mc_strtable_entry {
    const char *string;
    size_t size;
};

struct mc_strtable;

MORPHINE_API struct mc_strtable *mcapi_push_strtable(morphine_coroutine_t);
MORPHINE_API struct mc_strtable *mcapi_get_strtable(morphine_coroutine_t);
MORPHINE_API bool mcapi_strtable_has(struct mc_strtable *, mc_strtable_index_t);
MORPHINE_API mc_strtable_index_t mcapi_strtable_record(
    morphine_coroutine_t,
    struct mc_strtable *,
    const char *,
    size_t
);
MORPHINE_API struct mc_strtable_entry mcapi_strtable_access(
    morphine_coroutine_t,
    struct mc_strtable *,
    mc_strtable_index_t
);
