//
// Created by whyiskra on 30.12.23.
//

#pragma once

#include "morphine/platform.h"

void mlib_base_loader(morphine_coroutine_t);
void mlib_value_loader(morphine_coroutine_t);
void mlib_gc_loader(morphine_coroutine_t);
void mlib_coroutine_loader(morphine_coroutine_t);
void mlib_string_loader(morphine_coroutine_t);
void mlib_table_loader(morphine_coroutine_t);
void mlib_userdata_loader(morphine_coroutine_t);
void mlib_vector_loader(morphine_coroutine_t);
void mlib_registry_loader(morphine_coroutine_t);
void mlib_sio_loader(morphine_coroutine_t);
void mlib_bitwise_loader(morphine_coroutine_t);
