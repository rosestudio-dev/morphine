//
// Created by why-iskra on 30.03.2024.
//

#pragma once

#include "morphine/platform.h"

void init_env(morphine_instance_t);
void init_localstorage(morphine_instance_t);
void init_sharedstorage(morphine_instance_t);
void init_metatables(morphine_instance_t);
void init_sio(morphine_instance_t);
void init_libraries(morphine_instance_t);
void init_throw(morphine_instance_t);
