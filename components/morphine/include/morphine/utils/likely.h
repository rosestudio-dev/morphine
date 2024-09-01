//
// Created by whyiskra on 07.02.24.
//

#pragma once

#define likely(x) (__builtin_expect(((x) != 0), 1))
#define unlikely(x) (__builtin_expect(((x) != 0), 0))
