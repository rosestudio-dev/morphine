//
// Created by why-iskra on 31.07.2024.
//

#pragma once

#include <morphine.h>

#ifdef __cplusplus
extern "C" {
#endif

MORPHINE_API void mlapi_fs_file(morphine_coroutine_t, bool read, bool write, bool binary);

#ifdef __cplusplus
}
#endif
