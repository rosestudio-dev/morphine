//
// Created by why-iskra on 13.06.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_push_sio_buffer(morphine_coroutine_t, size_t factor, bool read, bool write);
MORPHINE_AUX void maux_sio_extract_string(morphine_coroutine_t);
