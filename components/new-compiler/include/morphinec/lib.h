//
// Created by why-iskra on 08.06.2024.
//

#pragma once

#include <morphine.h>

MORPHINE_LIB void mlib_compiler_loader(morphine_coroutine_t U);
MORPHINE_LIB void mlib_compiler_call(morphine_coroutine_t U, const char *name, size_t argc);