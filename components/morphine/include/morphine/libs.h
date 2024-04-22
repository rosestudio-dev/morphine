//
// Created by whyiskra on 25.12.23.
//

#pragma once

#include "platform.h"
#include "config.h"

MORPHINE_LIB void mlib_base_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_value_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_gc_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_coroutine_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_math_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_string_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_table_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_vector_call(morphine_coroutine_t, const char *name, size_t argc);
MORPHINE_LIB void mlib_registry_call(morphine_coroutine_t, const char *name, size_t argc);
