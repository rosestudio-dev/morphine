//
// Created by why on 1/17/25.
//

#pragma once

#include "morphine/config.h"
#include "morphine/core/throw.h"

#ifdef MORPHINE_ENABLE_DEBUG
    #define mm_assert_str2(v) #v
    #define mm_assert_str(v)  mm_assert_str2(v)
    #define mm_assert_line    mm_assert_str(__LINE__)

    #define mm_assert_error(I, m) (throwI_panic((I), "assert failed ("__FILE__": "mm_assert_line") '"m"'"))
    #define mm_assert(I, e, m)    ({ if (!(e)) { mm_assert_error((I), m); } (void) 0; })
#else
    #define mm_assert_error(I, m) ((void) 0)
    #define mm_assert(I, e, m)    ((void) 0)
#endif
