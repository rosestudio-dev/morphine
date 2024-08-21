//
// Created by whyiskra on 14.02.24.
//

#pragma once

// modules

#define MORPHINE_ENABLE_JUMPTABLE

// tags

#ifdef MORPHINE_LIBRARY
#define MORPHINE_API extern
#define MORPHINE_AUX extern
#define MORPHINE_LIB extern
#else
#define MORPHINE_API
#define MORPHINE_AUX
#define MORPHINE_LIB
#endif

// attrs

#define morphine_noret __attribute__((noreturn))
