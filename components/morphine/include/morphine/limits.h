//
// Created by why-iskra on 02.04.2024.
//

#pragma once

#include <stdint.h>
#include <inttypes.h>

// common

typedef int32_t ml_integer;
#define MLIMIT_INTEGER_PR  PRId32
#define MLIMIT_INTEGER_SC  SCNi32
#define MLIMIT_INTEGER_MAX INT32_MAX
#define MLIMIT_INTEGER_MIN INT32_MIN

typedef double ml_decimal;
#define MLIMIT_DECIMAL_PR "lg"
#define MLIMIT_DECIMAL_SC "lg"

// support

typedef uint32_t ml_size;
#define MLIMIT_SIZE_PR  PRIu32
#define MLIMIT_SIZE_SC  SCNu32
#define MLIMIT_SIZE_MAX INT32_MAX

// behaviour

#define MLIMIT_CALLABLE_ARGS   256
#define MLIMIT_CALLABLE_PARAMS 65535
#define MLIMIT_CALLABLE_SLOTS  65535
#define MLIMIT_NATIVE_NAME     1024
#define MLIMIT_FUNCTION_NAME   1024
#define MLIMIT_USERDATA_NAME   1024

// checks

_Static_assert(sizeof(ml_size) <= sizeof(size_t), "ml_size incompatible with arch");
_Static_assert(sizeof(ml_size) <= sizeof(ml_integer), "ml_size incompatible with ml_integer");
_Static_assert(sizeof(size_t) <= sizeof(intmax_t) && 4 <= sizeof(size_t), "incompatible arch");
