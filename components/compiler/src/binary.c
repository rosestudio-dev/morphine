//
// Created by why-iskra on 12.06.2024.
//

#include "morphinec/binary.h"
#include "binary/impl.h"

MORPHINE_API void mcapi_to_binary(morphine_coroutine_t U) {
    binary_to(U);
}

MORPHINE_API void mcapi_from_binary(morphine_coroutine_t U) {
    binary_from(U);
}
