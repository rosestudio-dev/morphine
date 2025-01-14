//
// Created by why on 1/14/25.
//

#pragma once

#include <morphine.h>

#define MLIB_PROCESS_USERDATA_TYPE "process"

MORPHINE_API void mlapi_process_spawn(morphine_coroutine_t, bool env);
MORPHINE_API void mlapi_process_kill(morphine_coroutine_t, bool force);
MORPHINE_API int mlapi_process_wait(morphine_coroutine_t);
MORPHINE_API bool mlapi_process_isalive(morphine_coroutine_t);
