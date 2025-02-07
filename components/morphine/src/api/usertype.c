//
// Created by why on 2/6/25.
//

#include "morphine/core/usertype.h"
#include "morphine/api.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_usertype_declare(morphine_coroutine_t U, morphine_usertype_t usertype) {
    struct table *table = usertype.metatable ? valueI_as_table_or_error(U->I, stackI_peek(U, 0)) : NULL;

    usertypeI_declare(
        U->I,
        usertype.name,
        usertype.size,
        usertype.constructor,
        usertype.destructor,
        table
    );

    if (usertype.metatable) {
        stackI_pop(U, 1);
    }
}

MORPHINE_API bool mapi_usertype_has(morphine_coroutine_t U, const char *name) {
    return usertypeI_has(U->I, name);
}
