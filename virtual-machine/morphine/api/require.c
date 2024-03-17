//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/require.h"
#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_require(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = requireI_load(S, *value);
}

MORPHINE_API void mapi_require_get(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 2);
    *value = requireI_load(S, *value);
    *value = tableI_get(S->I, valueI_as_table_or_error(S, *value), value[1], NULL);
    stackI_pop(S, 1);
}
