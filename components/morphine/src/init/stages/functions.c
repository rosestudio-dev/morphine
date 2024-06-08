//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "functions/require.h"
#include "morphine/core/instance.h"
#include "morphine/misc/registry.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"

static void init_require(morphine_instance_t I) {
    struct value name = valueI_object(stringI_create(I, "require"));
    struct value native = valueI_object(nativeI_create(I, "require", require));
    registryI_set_key(I, native, name);
    tableI_set(I, I->env, name, native);
}

void init_functions(morphine_instance_t I) {
    init_require(I);
}