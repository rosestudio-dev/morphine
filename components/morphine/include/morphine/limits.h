//
// Created by why-iskra on 02.04.2024.
//

#pragma once

#include <stdint.h>
#include <inttypes.h>

typedef int32_t ml_integer;
#define MLIMIT_INTEGER_PR  PRId32
#define MLIMIT_INTEGER_SC  SCNi32
#define MLIMIT_INTEGER_MAX INT32_MAX
#define MLIMIT_INTEGER_MIN INT32_MIN

typedef double ml_decimal;
#define MLIMIT_DECIMAL_PR "lg"
#define MLIMIT_DECIMAL_SC "lg"
