//
// Created by why-iskra on 13.06.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_push_sio_buffer(morphine_coroutine_t, size_t factor, bool read, bool write);
MORPHINE_AUX void maux_sio_extract_string(morphine_coroutine_t);

MORPHINE_AUX morphine_sio_interface_t maux_sio_interface_srw(morphine_sio_read_t, morphine_sio_write_t);
#define maux_sio_interface_sro(f)     maux_sio_interface_srw((f), NULL)
#define maux_sio_interface_swo(f)     maux_sio_interface_srw(NULL, (f))
#define maux_sio_interface_stub()     maux_sio_interface_srw(NULL, NULL)
