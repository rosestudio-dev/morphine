//
// Created by why-iskra on 10.11.2024.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "morphine/attrs.h"

#define MBIT_CNTRL  0
#define MBIT_SPACE  1
#define MBIT_BLANK  2
#define MBIT_PUNCT  3
#define MBIT_ALPHA  4
#define MBIT_UPPER  5
#define MBIT_DIGIT  6
#define MBIT_XDIGIT 7

#define morphine_ctype_testprob(c, b) ((morphine_ctype_probs[(uint8_t) (c)] & (1 << (b))) == (1 << (b)))

#define morphine_iscntrl(c)  (morphine_ctype_testprob((c), MBIT_CNTRL))
#define morphine_isspace(c)  (morphine_ctype_testprob((c), MBIT_SPACE))
#define morphine_isblank(c)  (morphine_ctype_testprob((c), MBIT_BLANK))
#define morphine_ispunct(c)  (morphine_ctype_testprob((c), MBIT_PUNCT))
#define morphine_isalpha(c)  (morphine_ctype_testprob((c), MBIT_ALPHA))
#define morphine_isupper(c)  (morphine_ctype_testprob((c), MBIT_UPPER))
#define morphine_isdigit(c)  (morphine_ctype_testprob((c), MBIT_DIGIT))
#define morphine_isxdigit(c) (morphine_ctype_testprob((c), MBIT_XDIGIT))

#define morphine_toupper(c) (morphine_isalpha((c)) && !morphine_isupper((c)) ? ((c) - ('a' - 'A')) : (c))
#define morphine_tolower(c) (morphine_isalpha((c)) && morphine_isupper((c)) ? ((c) + ('a' - 'A')) : (c))

morphine_export const uint8_t morphine_ctype_probs[UCHAR_MAX + 1];

static inline bool morphine_isprint(char c) { return !morphine_iscntrl(c); }
static inline bool morphine_isgraph(char c) { return morphine_isprint(c) && !morphine_isspace(c); }
static inline bool morphine_isalnum(char c) { return morphine_isalpha(c) || morphine_isdigit(c); }
static inline bool morphine_islower(char c) { return morphine_isalpha(c) && !morphine_isupper(c); }
