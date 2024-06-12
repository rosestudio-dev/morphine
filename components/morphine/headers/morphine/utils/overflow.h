//
// Created by why-iskra on 11.06.2024.
//

#pragma once

#include "likely.h"

#define overflow_condition_add(a, b, max) ((a) > ((max) - (b)))
#define overflow_condition_sub(a, b, min) ((a) < ((b) + (min)))
#define overflow_condition_mul(a, b, max) ((b) != 0 && (a) > (max) / (b))

#define overflow_add(a, b, max) if(unlikely(overflow_condition_add(a, b, max)))
#define overflow_sub(a, b, min) if(unlikely(overflow_condition_sub(a, b, min)))
#define overflow_mul(a, b, max) if(unlikely(overflow_condition_mul(a, b, max)))
