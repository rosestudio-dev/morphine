//
// Created by why-iskra on 02.04.2024.
//

#pragma once

#include "utils/overflow.h"
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

// common

typedef int64_t ml_integer;
typedef double ml_decimal;
typedef uint32_t ml_size;

// misc

typedef uint32_t ml_version;
typedef uint16_t ml_argument;
typedef uint32_t ml_line;
typedef uint32_t ml_hash;

// prints

#define MLIMIT_INTEGER_PR   PRId64
#define MLIMIT_DECIMAL_PR   "lg"
#define MLIMIT_SIZE_PR      PRIu32

#define MLIMIT_VERSION_PR   PRIu32
#define MLIMIT_ARGUMENT_PR  PRIu16
#define MLIMIT_LINE_PR      PRIu32
#define MLIMIT_HASH_PR      PRIu32

// checks

static_assert(4 <= sizeof(size_t), "incompatible arch");

static_assert(mm_typemax(ml_size) <= SIZE_MAX, "ml_size incompatible with arch");
static_assert(mm_typemax(ml_size) <= mm_typemax(ml_integer), "ml_size incompatible with ml_integer");

static_assert(mm_typemax(ml_version) <= mm_typemax(ml_integer), "ml_version incompatible with ml_integer");
static_assert(mm_typemax(ml_argument) <= mm_typemax(ml_size), "ml_argument incompatible with ml_size");
static_assert(mm_typemax(ml_line) <= mm_typemax(ml_integer), "ml_line incompatible with ml_integer");
static_assert(sizeof(ml_hash) <= sizeof(ml_integer), "ml_hash incompatible with ml_integer");
