//
// Created by whyiskra on 30.12.23.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_LIB morphine_library_t *mlib_builtin_base(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_value(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_gc(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_coroutine(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_string(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_table(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_userdata(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_vector(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_registry(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_sio(void);
MORPHINE_LIB morphine_library_t *mlib_builtin_bitwise(void);
