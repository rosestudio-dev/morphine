//
// Created by why-iskra on 13.06.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_push_sio_buffer(morphine_coroutine_t, size_t factor, bool read, bool write);
MORPHINE_AUX void maux_sio_read_all(morphine_coroutine_t);
MORPHINE_AUX void maux_sio_read_to(morphine_coroutine_t, const char *exit);
MORPHINE_AUX void maux_sio_read_line(morphine_coroutine_t);

MORPHINE_AUX morphine_sio_interface_t maux_sio_interface_srwf(
    morphine_sio_read_t,
    morphine_sio_write_t,
    morphine_sio_flush_t
);

#define maux_sio_interface_sro(f)    maux_sio_interface_srwf((f), NULL, NULL)
#define maux_sio_interface_swo(f)    maux_sio_interface_srwf(NULL, (f), NULL)
#define maux_sio_interface_srw(r, w) maux_sio_interface_srwf((r), (w), NULL)
#define maux_sio_interface_swf(w, f) maux_sio_interface_srwf(NULL, (w), (f))
#define maux_sio_interface_stub()    maux_sio_interface_srwf(NULL, NULL, NULL)
