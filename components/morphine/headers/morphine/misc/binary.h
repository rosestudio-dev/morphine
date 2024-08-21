//
// Created by why-iskra on 21.08.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/object/sio.h"

#define FORMAT_TAG "morphine-binary"
#define MORPHINEC_BINARY_VERSION 1

#define PROB_INTEGER (-201427)
#define PROB_SIZE    (201427)
#define PROB_DECIMAL (1548.5629)

void binaryI_to(morphine_instance_t I, struct sio *sio, struct value value);
struct value binaryI_from(morphine_instance_t I, struct sio *sio);
