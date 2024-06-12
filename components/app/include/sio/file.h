//
// Created by why-iskra on 12.06.2024.
//

#pragma once

#include <morphine.h>

void sio_file(
    morphine_coroutine_t,
    const char *path,
    bool read,
    bool write,
    bool binary
);
