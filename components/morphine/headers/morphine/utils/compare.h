//
// Created by why-iskra on 06.10.2024.
//

#pragma once

#include <stddef.h>

#define smpcmp(a, b) ((a) == (b) ? 0 : ((a) < (b) ? -1 : 1))
int arrcmp(const void *, const void *, size_t, size_t);
