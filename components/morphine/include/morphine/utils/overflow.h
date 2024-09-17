//
// Created by why-iskra on 11.06.2024.
//

#pragma once

#include "likely.h"

#define overflow_condition_add(a, b, max)        ((a) > ((max) - (b)))
#define overflow_condition_sub(a, b, min)        ((a) < ((b) + (min)))
#define overflow_condition_unsafe_mul(a, b, max) ((a) > ((max) / (b)))
#define overflow_condition_mul(a, b, max)        (((b) != 0) && overflow_condition_unsafe_mul(a, b, max))

#define overflow_condition_signed_add(a, b, min, max) (((a) > 0 && (b) > 0 && overflow_condition_add(a, b, max)) || ((a) < 0 && (b) < 0 && overflow_condition_add(-(a), -(b), min)))
#define overflow_condition_signed_sub(a, b, min, max) (((a) > 0 && (b) < 0 && overflow_condition_sub(-(a), -(b), max)) || ((a) < 0 && (b) > 0 && overflow_condition_sub(a, b, min)))
#define overflow_condition_signed_mul(a, b, min, max) ( \
    (((a) > 0) && ((b) > 0) && overflow_condition_unsafe_mul(a, b, max)) || \
    (((a) > 0) && ((b) < 0) && overflow_condition_unsafe_mul(b, a, min)) || \
    (((a) < 0) && ((b) > 0) && overflow_condition_unsafe_mul(a, b, min)) || \
    (((a) < 0) && ((b) < 0) && overflow_condition_unsafe_mul(b, a, max)) \
)

#define overflow_add(a, b, max) if(unlikely(overflow_condition_add(a, b, max)))
#define overflow_sub(a, b, min) if(unlikely(overflow_condition_sub(a, b, min)))
#define overflow_mul(a, b, max) if(unlikely(overflow_condition_mul(a, b, max)))

#define overflow_signed_add(a, b, min, max) if(unlikely(overflow_condition_signed_add(a, b, min, max)))
#define overflow_signed_sub(a, b, min, max) if(unlikely(overflow_condition_signed_sub(a, b, min, max)))
#define overflow_signed_mul(a, b, min, max) if(unlikely(overflow_condition_signed_mul(a, b, min, max)))

#define overflow_op_add(a, b, max, code) ({if(unlikely(overflow_condition_add(a, b, max))) { code; }; a + b;})
#define overflow_op_sub(a, b, min, code) ({if(unlikely(overflow_condition_sub(a, b, min))) { code; }; a - b;})
#define overflow_op_mul(a, b, max, code) ({if(unlikely(overflow_condition_mul(a, b, max))) { code; }; a * b;})

#define overflow_op_signed_add(a, b, min, max) ({if(unlikely(overflow_condition_signed_add(a, b, min, max))) { code; }; a + b;})
#define overflow_op_signed_sub(a, b, min, max) ({if(unlikely(overflow_condition_signed_sub(a, b, min, max))) { code; }; a - b;})
#define overflow_op_signed_mul(a, b, min, max) ({if(unlikely(overflow_condition_signed_mul(a, b, min, max))) { code; }; a * b;})
