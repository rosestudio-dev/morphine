//
// Created by why on 1/15/25.
//

#pragma once

// modifiers

#define morphine_noret        __attribute__((noreturn))
#define morphine_unused       __attribute__((unused))
#define morphine_printf(f, a) __attribute__((format (printf, (f), (a))))
#define morphine_export       extern __attribute__((visibility ("default")))

// tags

#ifdef MORPHINE_LIBRARY
    #define MORPHINE_API morphine_export
    #define MORPHINE_AUX morphine_export
    #define MORPHINE_LIB morphine_export
#else
    #define MORPHINE_API
    #define MORPHINE_AUX
    #define MORPHINE_LIB
#endif
