//
// Created by whyiskra on 07.02.24.
//

#pragma once

#define morphinem_likely(x) (__builtin_expect(((x) != 0), 1))
#define morphinem_unlikely(x) (__builtin_expect(((x) != 0), 0))
