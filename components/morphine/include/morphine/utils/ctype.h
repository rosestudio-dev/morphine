//
// Created by why-iskra on 10.11.2024.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "morphine/attrs.h"

#define MM_CTYPE_BIT_CNTRL  0
#define MM_CTYPE_BIT_SPACE  1
#define MM_CTYPE_BIT_BLANK  2
#define MM_CTYPE_BIT_PUNCT  3
#define MM_CTYPE_BIT_ALPHA  4
#define MM_CTYPE_BIT_UPPER  5
#define MM_CTYPE_BIT_DIGIT  6
#define MM_CTYPE_BIT_XDIGIT 7

#define mm_ctype_testprob(c, b) ((morphine_ctype_probs[(uint8_t) (c)] & (1 << (b))) == (1 << (b)))

#define mm_ctype_iscntrl(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_CNTRL))
#define mm_ctype_isspace(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_SPACE))
#define mm_ctype_isblank(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_BLANK))
#define mm_ctype_ispunct(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_PUNCT))
#define mm_ctype_isalpha(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_ALPHA))
#define mm_ctype_isupper(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_UPPER))
#define mm_ctype_isdigit(c)  (mm_ctype_testprob((c), MM_CTYPE_BIT_DIGIT))
#define mm_ctype_isxdigit(c) (mm_ctype_testprob((c), MM_CTYPE_BIT_XDIGIT))

mattr_export const uint8_t morphine_ctype_probs[UCHAR_MAX + 1];

static inline bool mm_ctype_isprint(char c) { return !mm_ctype_iscntrl(c); }
static inline bool mm_ctype_isgraph(char c) { return mm_ctype_isprint(c) && !mm_ctype_isspace(c); }
static inline bool mm_ctype_isalnum(char c) { return mm_ctype_isalpha(c) || mm_ctype_isdigit(c); }
static inline bool mm_ctype_islower(char c) { return mm_ctype_isalpha(c) && !mm_ctype_isupper(c); }

static inline char mm_ctype_toupper(char c) { return (char) (mm_ctype_islower(c) ? c - ('a' - 'A') : c); }
static inline char mm_ctype_tolower(char c) { return (char) (mm_ctype_isupper(c) ? c + ('a' - 'A') : c); }
