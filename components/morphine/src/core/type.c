//
// Created by why-iskra on 14.12.2024.
//

#include "morphine/core/type.h"
#include "morphine/core/throw.h"

const char *typeI_tostring(morphine_instance_t I, enum value_type type) {
    switch (type) {
#define type_case(n, s)            case VALUE_TYPE_##n: return #s;
#define mspec_type_object(i, n, s) type_case(n, s)
#define mspec_type_value(i, n, s)  type_case(n, s)

#include "morphine/core/type/specification.h"

#undef type_case
#undef mspec_type_object
#undef mspec_type_value
    }

    throwI_panic(I, "unsupported type");
}

bool typeI_isobj(enum value_type type) {
#define mspec_type_object(i, n, s) if (type == VALUE_TYPE_##n) { return true; };
#define mspec_type_value(i, n, s)

#include "morphine/core/type/specification.h"

#undef mspec_type_object
#undef mspec_type_value

    return false;
}
