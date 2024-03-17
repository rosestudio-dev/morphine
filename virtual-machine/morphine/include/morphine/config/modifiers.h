//
// Created by whyiskra on 16.12.23.
//

#pragma once

#ifdef MORPHINE_LIBRARY_VERSION
#define MORPHINE_API extern
#else
#define MORPHINE_API
#endif

#ifdef MORPHINE_LIBRARY_VERSION
#define MORPHINE_AUX extern
#else
#define MORPHINE_AUX
#endif

#ifdef MORPHINE_LIBRARY_VERSION
#define MORPHINE_LIB extern
#else
#define MORPHINE_LIB
#endif
