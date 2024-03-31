//
// Created by whyiskra on 14.02.24.
//

#pragma once

// modules

#define MORPHINE_ENABLE_JUMPTABLE
//#define MORPHINE_ENABLE_DEBUGGER

// tags

#ifdef MORPHINE_LIBRARY
#define MORPHINE_API extern
#else
#define MORPHINE_API
#endif

#ifdef MORPHINE_LIBRARY
#define MORPHINE_AUX extern
#else
#define MORPHINE_AUX
#endif

#ifdef MORPHINE_LIBRARY
#define MORPHINE_LIB extern
#else
#define MORPHINE_LIB
#endif

// attrs

#define morphine_noret __attribute__((noreturn))
