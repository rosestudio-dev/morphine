//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/object/coroutine/callstack.h"

void gcI_dispose_callinfo(morphine_instance_t, struct callinfo *callinfo);
struct callinfo *gcI_hot_callinfo(morphine_instance_t);
