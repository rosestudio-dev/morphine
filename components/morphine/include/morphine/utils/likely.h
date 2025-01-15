//
// Created by whyiskra on 07.02.24.
//

#pragma once

#define mm_likely(x)   (__builtin_expect(((x) != 0), 1))
#define mm_unlikely(x) (__builtin_expect(((x) != 0), 0))
