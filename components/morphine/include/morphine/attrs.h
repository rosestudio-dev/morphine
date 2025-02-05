//
// Created by why on 1/15/25.
//

#pragma once

// modifiers

#define mattr_noret        __attribute__((noreturn))
#define mattr_unused       __attribute__((unused))
#define mattr_printf(f, a) __attribute__((format (printf, (f), (a))))
#define mattr_export       extern __attribute__((visibility ("default")))

// tags

#ifdef MORPHINE_LIBRARY
    #define MORPHINE_API mattr_export
    #define MORPHINE_AUX mattr_export
    #define MORPHINE_LIB mattr_export
#else
    #define MORPHINE_API
    #define MORPHINE_AUX
    #define MORPHINE_LIB
#endif
