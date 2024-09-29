//
// Created by why-iskra on 02.09.2024.
//

#pragma once

#include <morphine.h>

#define MLIB_BIGINT_USERDATA_TYPE "bigint"

struct mlib_bigint;

MORPHINE_API struct mlib_bigint *mlapi_get_bigint(morphine_coroutine_t);
MORPHINE_API struct mlib_bigint *mlapi_bigint_from_integer(morphine_coroutine_t, ml_integer);
MORPHINE_API struct mlib_bigint *mlapi_bigint_from_string(morphine_coroutine_t, const char *);
MORPHINE_API int mlapi_bigint_compare(struct mlib_bigint *, struct mlib_bigint *);
MORPHINE_API void mlapi_bigint_tostring(morphine_coroutine_t, struct mlib_bigint *);

MORPHINE_API struct mlib_bigint *mlapi_bigint_add(
    morphine_coroutine_t,
    struct mlib_bigint *,
    struct mlib_bigint *,
    struct mlib_bigint *
);

MORPHINE_API struct mlib_bigint *mlapi_bigint_sub(
    morphine_coroutine_t,
    struct mlib_bigint *,
    struct mlib_bigint *,
    struct mlib_bigint *
);

MORPHINE_API struct mlib_bigint *mlapi_bigint_mul(
    morphine_coroutine_t,
    struct mlib_bigint *,
    struct mlib_bigint *,
    struct mlib_bigint *
);

MORPHINE_API struct mlib_bigint *mlapi_bigint_div(
    morphine_coroutine_t,
    struct mlib_bigint *,
    struct mlib_bigint *,
    struct mlib_bigint *
);

MORPHINE_API struct mlib_bigint *mlapi_bigint_mod(
    morphine_coroutine_t,
    struct mlib_bigint *,
    struct mlib_bigint *,
    struct mlib_bigint *
);

MORPHINE_API struct mlib_bigint *mlapi_bigint_negate(morphine_coroutine_t, struct mlib_bigint *, bool);
