//
// Created by whyiskra on 14.02.24.
//

#pragma once

// modules

#define MORPHINE_ENABLE_JUMPTABLE
#define MORPHINE_ENABLE_SSO

// modifiers

#define morphine_noret  __attribute__((noreturn))
#define morphine_dynlib extern

// tags

#ifdef MORPHINE_LIBRARY
#define MORPHINE_API morphine_dynlib
#define MORPHINE_AUX morphine_dynlib
#define MORPHINE_LIB morphine_dynlib
#else
#define MORPHINE_API
#define MORPHINE_AUX
#define MORPHINE_LIB
#endif
