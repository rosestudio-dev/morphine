//
// Created by why-iskra on 12.06.2024.
//

#pragma once

#include <morphine.h>

#define MORPHINEC_BINARY_VERSION 1

MORPHINE_API ml_version mcapi_binary_version(void);
MORPHINE_API void mcapi_to_binary(morphine_coroutine_t);
MORPHINE_API void mcapi_from_binary(morphine_coroutine_t);
