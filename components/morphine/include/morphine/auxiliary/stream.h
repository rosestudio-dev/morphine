//
// Created by why-iskra on 13.06.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_push_stream_buffer(morphine_coroutine_t, size_t factor);
MORPHINE_AUX void maux_push_stream_empty(morphine_coroutine_t, bool read_eof, bool write_eof);

MORPHINE_AUX void maux_stream_read_all(morphine_coroutine_t);
MORPHINE_AUX bool maux_stream_read_to(morphine_coroutine_t, const char *exit, bool eof);
MORPHINE_AUX bool maux_stream_read_line(morphine_coroutine_t);

MORPHINE_AUX morphine_stream_interface_t maux_stream_interface_srwf(
    morphine_stream_read_t,
    morphine_stream_write_t,
    morphine_stream_flush_t
);

#define maux_stream_interface_sro(f)    maux_stream_interface_srwf((f), NULL, NULL)
#define maux_stream_interface_swo(f)    maux_stream_interface_srwf(NULL, (f), NULL)
#define maux_stream_interface_srw(r, w) maux_stream_interface_srwf((r), (w), NULL)
#define maux_stream_interface_swf(w, f) maux_stream_interface_srwf(NULL, (w), (f))
#define maux_stream_interface_stub()    maux_stream_interface_srwf(NULL, NULL, NULL)
