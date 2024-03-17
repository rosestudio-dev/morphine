//
// Created by whyiskra on 07.02.24.
//

#pragma once

#define morphinem_private_macros_to_str(x) morphinem_private_macros_to_str2(x)
#define morphinem_private_macros_to_str2(x) #x
#define morphinem_private_linestr morphinem_private_macros_to_str(__LINE__)

#define morphinem_static_assert(t, c) _Static_assert(!(c), "File: " __FILE__ " Line: " morphinem_private_linestr " " #t)
