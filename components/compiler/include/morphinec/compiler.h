//
// Created by why-iskra on 08.06.2024.
//

#pragma once

#include <morphine.h>

MORPHINE_API void mcapi_compile(
    morphine_coroutine_t,
    const char *main,
    const char *text,
    size_t main_size,
    size_t text_size
);
